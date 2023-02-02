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
using TimePoint = std::chrono::time_point<std::chrono::system_clock>;


const string Controller::cv_window_name = "Tello camera";

void Controller::get_battery_stat() {
	while (true) {
		this->battery_stat = this->tello.get_battery();
		std::this_thread::sleep_for(milliseconds(WAIT_BATTERY));
	}
}

void Controller::run() {
	// log
	std::thread control_thread(&Controller::send_command, this);
	control_thread.detach();
	std::thread battery_thread(&Controller::get_battery_stat, this);
	battery_thread.detach();

	// log
	cv::Mat image;
	cv::namedWindow(cv_window_name);
	cv::VideoCapture cap(0);
	if (!cap.isOpened()) {
		// log
		throw std::exception("Unable to open video stream");
	}
	while (true) {
		cap >> image;
		this->detection_step(&image);
		cv::imshow(cv_window_name, image);
		if (cv::waitKey(25) == 27 ||
			cv::getWindowProperty(cv_window_name, cv::WND_PROP_VISIBLE) < 1
			) {
			cv::destroyAllWindows();
			// REVIEW threads clean exit
			// TODO tello clean-up
			break;
		}

	}
	// TODO catch Ctrl+C KeyboardInterrupt (<csignal>?)
}

//        try:
//            while True:
//                img = self.frame_read.frame
//                if img is None:
//                    continue
//                img = self.step(img)
//                if self.save_video:
//                    writer.write(img)
//                cv2.imshow(self.CAMERA_NAME, img)
//
//                if cv2.waitKey(1) == 27 or (
//                    int(
//                        cv2.getWindowProperty(
//                            self.CAMERA_NAME, cv2.WND_PROP_VISIBLE
//                        )
//                    )
//                    < 1
//                ):
//                    timer_bat.cancel()
//                    timer_com.cancel()
//                    if not self.debug:
//                        self.tello.land()
//                    if self.save_video:
//                        writer.release()
//                    self.tello.streamoff()
//                    break
//        except KeyboardInterrupt:
//            self.logger.info("KeyboardInterrupt: exiting")
//            timer_bat.cancel()
//            timer_com.cancel()
//            if not self.debug:
//                self.tello.land()
//            if self.save_video:
//                        writer.release()
//            self.tello.streamoff()

void Controller::detection_step(cv::Mat* img) {
	this->_put_battery_on_frame(img);

	Detection face_detection = this->face_detector.detect(*img);
	if (face_detection.box.score > 0) {
		this->_last_face = system_clock::now();
		// TODO crop image for gesture detection
		cv::Mat cropped = *img;
		this->face_detector.visualize(img, face_detection);
		GestureDetection gesture_detection = this->gesture_detector.detect(cropped);
		if (gesture_detection.box.score > 0) {
			this->_last_gesture = system_clock::now();
			this->stop_tello = false;
			this->buffer.add(gesture_detection.gesture);
			this->gesture_detector.visualize(img, gesture_detection);
		}
		// TODO draw bounds
	}
}

void Controller::send_command() {
	while (true) {
		if (!this->stop_tello) {
			if ((system_clock::now() - this->_last_face) > this->FACE_TIMEOUT ||
				(system_clock::now() - this->_last_gesture) > this->GESTURE_TIMEOUT) {
				this->stop();
			}
			else {
				Gesture gesture = this->buffer.get();
				if (gesture != NoGesture) {
					// log
				}
				if (!this->is_landing) {
					vector<int> vel = { -1, -1, -1, -1 };
					switch (gesture)
					{
					case NoGesture:
						break;
					case Stop:
						this->stop();
						break;
					case Left:
						vel = { -dw[0], 0, 0, 0 };
						break;
					case Right:
						vel = { dw[0], 0, 0, 0 };
						break;
					case Up:
						vel = { 0, 0, dw[2], 0 };
						break;
					case Down:
						vel = { 0, 0, -dw[2], 0 };
						break;
					case Forward:
						vel = { 0, dw[1], 0, 0 };
						break;
					case Back:
						vel = { 0, -dw[1], 0, 0 };
						break;
					case Land:
						this->tello.land();
						this->is_landing = true;
						break;
					default:
						break;
					}
				}
				if (vel.at(3) != -1) {
					// log
					this->vel = vel;
					if (!this->debug) {
						this->tello.send_rc_control(vel);
					}
				}
			}
		}
		std::this_thread::sleep_for(milliseconds(WAIT_RC_CONTROL));
	}
}

void Controller::stop() {
	this->vel = { 0, 0, 0, 0 };
	this->stop_tello = true;
	this->tello.send_rc_control(this->vel);
}

void Controller::_put_battery_on_frame(cv::Mat* img) {
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
			_buffer.clear();
			return val;
		}
		else {
			T val{};
			return val;
		}
}

void Tello::send_rc_control(const vector<int>& vel) {
	// log
	std::cout << "rc ";
	for (auto vel_i : vel) {
		std::cout << vel_i << " ";
	}
	std::cout << std::endl;
}

void Tello::land() {
	// log
}
