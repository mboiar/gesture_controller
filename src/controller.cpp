#include <thread>
#include <iostream>
#include <stdexcept>
#include <atomic>
#include <map>
#include <algorithm>
//#include <csignal>

#include "controller.h"

using std::chrono::system_clock;
using std::chrono::milliseconds;
using std::string;
using std::vector;
using std::atomic;
using TimePoint = std::chrono::time_point<std::chrono::system_clock>;
using namespace std::chrono_literals;

const string Controller::cv_window_name = "Tello camera";

void Controller::get_battery_stat() {
	// modifies battery_stat
	while (true) {
		this->battery_stat = tello->get_battery();
		std::this_thread::sleep_for(milliseconds(WAIT_BATTERY));
	}
}

void Controller::run() {
	// log
	std::thread control_thread(&Controller::send_command, this);
	control_thread.detach();
	std::thread battery_thread(&Controller::get_battery_stat, this);
	battery_thread.detach();

	cv::VideoCapture cap = tello->get_video_stream();
	cv::Mat frame1;
	cv::namedWindow(cv_window_name);

	double frame_count = 0;
	TimePoint start_time = system_clock::now();
	TimePoint end_time;
	double fps = 0;
	logger.info("Starting detection");

	while (true) {
		cap >> frame1;
		if (frame1.empty()) {
			// log
			continue;
		}
		// get fps
		if (++frame_count >= 10) {
			end_time = system_clock::now();
			fps = frame_count / ((end_time - start_time)/1.0s);
			start_time = end_time;
			frame_count = 0;
		}
		this->detection_step(&frame1);

		// put fps
		cv::putText(frame1, std::to_string((int)fps)+" fps", cv::Point(20, 50), 1, 2, (0, 255, 255), 2);
		// put battery_stat
		this->_put_battery_on_frame(&frame1);
		
		cv::imshow(cv_window_name, frame1);

		char key = (char)cv::waitKey(10);
		if (key == 27 || key == 'q' ||
			cv::getWindowProperty(cv_window_name, cv::WND_PROP_VISIBLE) < 1
			) {
			//cv::destroyAllWindows();
			// REVIEW threads clean exit
			// TODO tello clean-up
			break;
		}
	}
	// TODO catch Ctrl+C KeyboardInterrupt (<csignal>?)
}

void Controller::detection_step(cv::Mat* img) {
	// sets _last_face, _last_gesture, stop_tello, buffer
	// accesses face_detector, gesture_detector

	Detection face_detection = this->face_detector.detectAndDisplay(*img);
	if (face_detection.score > 0) {
		this->_last_face = system_clock::now();
		cv::Scalar color = cv::Scalar(0, 0, 255);

		this->face_detector.visualize(img, face_detection);
		cv::Rect gesture_box = this->face_detector.generate_bounding_box(face_detection.box, *img, 200, 200);
		cv::rectangle(*img, gesture_box, color, 2);

		cv::Mat ROI(*img, gesture_box);
		GestureDetection gesture_detection = this->gesture_detector.detect(ROI);
		if (gesture_detection.score > 0) {
			this->_last_gesture = system_clock::now();
			this->stop_tello = false;
			this->buffer.add(gesture_detection.gesture);
			this->gesture_detector.visualize(img, gesture_detection, gesture_box);
		}
		// TODO draw bounds
	}
}

void Controller::send_command() {
	// accesses _last_face, _last_gesture, stop, buffer, is_landing, tello, vel, debug, stop_tello
	// sets vel, buffer

	while (true) {
		if (!this->stop_tello) {
			if ((system_clock::now() - this->_last_face) > FACE_TIMEOUT ||
				(system_clock::now() - this->_last_gesture) > GESTURE_TIMEOUT) {
				spdlog::info("No face or gesture: stopping Tello");
				this->stop();
			}
			else {
				vector<int> vel = { 0, 0, 0, -1 };
				Gesture gesture = this->buffer.get();

				if (gesture != NoGesture) {
					spdlog::info("Received gesture {}", gesture);
					if (!this->is_landing) {
						switch (gesture)
						{
						case NoGesture:
							break;
						case Stop:
							this->stop();
							break;
						case Left:
							vel.at(0) = -1*dw[0];
							vel.at(3) = 0;
							break;
						case Right:
							vel.at(0) = dw[0];
							vel.at(3) = 0;
							break;
						case Up:
							vel.at(2) = dw[2];
							vel.at(3) = 0;
							break;
						case Down:
							vel.at(2) = -1*dw[2];
							vel.at(3) = 0;
							break;
						case Forward:
							vel.at(1) = dw[1];
							vel.at(3) = 0;
							break;
						case Back:
							vel.at(1) = -1*dw[1];
							vel.at(3) = 0;
							break;
						case Land:
							tello->land();
							this->is_landing = true;
							break;
						default:
							break;
						}
					}

					if (vel.at(3) != -1 && this->vel != vel) {
						this->vel = vel;
						if (!this->debug) {
							tello->send_rc_control(vel);
						}
					}
				}
			}
		}
		std::this_thread::sleep_for(milliseconds(WAIT_RC_CONTROL));
	}
}

void Controller::stop() {
	// modifies vel, stop_tello
	this->vel = { 0, 0, 0, 0 };
	this->stop_tello = true;
	tello->send_rc_control(this->vel);
}

void Controller::_put_battery_on_frame(cv::Mat* img) {
	// accesses battery_stat
	string text("No battery info");
	if (this->battery_stat > 0) {
		text = std::to_string(this->battery_stat) + "%";
	}
	cv::putText(*img, text, cv::Point(20, 100), 1, 2, (0, 255, 255), 2);
}



template<class T>
void Buffer<T>::add(const T& elem) {
	this->_buffer.at(elem)++;
}

template <class T>
T Buffer<T>::get() {
		// REVIEW actual buffer
		auto max_count = std::max_element(_buffer.begin(), _buffer.end());
		if (max_count != _buffer.end() && *max_count >= max_len) {
			T val = static_cast<T>(std::distance(_buffer.begin(), max_count));
			//std::cout << *max_count << std::endl;
			_buffer.assign(size, 0);
			return val;
		}
		else {
			return (T)0;
		}
}

const char Tello::TELLO_STREAM_URL[] = "udp://0.0.0.0:11111";

void Tello::send_rc_control(const vector<int>& vel) {
	spdlog::info("rc {} {} {} {}", vel.at(0), vel.at(1), vel.at(2), vel.at(3));
	//for (auto vel_i : vel) {
	//	std::cout << vel_i << " ";
	//}
	//std::cout << std::endl;
}

void Tello::land() {
	spdlog::info("land");
}

int Tello::get_battery() {
	spdlog::info("Battery: {}%", 100);
	return 100;
}

cv::VideoCapture Tello::get_video_stream() {
	cv::VideoCapture cap;
	if (simulate) {
		cap = cv::VideoCapture(0);
	}
	else {
		cap = cv::VideoCapture(TELLO_STREAM_URL, cv::CAP_FFMPEG);
	}
	if (!cap.isOpened()) {
		spdlog::error("Unable to get video stream");
		// TODO handle error
	}
	return cap;
}