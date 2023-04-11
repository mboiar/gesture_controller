#include <thread>
#include <iostream>
#include <stdexcept>
#include <atomic>
#include <map>
#include <algorithm>
#include <regex>
#include<fstream>
//#include <csignal>
#include "controller.h"
//#include "opencv2/core_detect.hpp"
using namespace cv;

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
	cv::Mat frame;
	cv::namedWindow(cv_window_name);

	double frame_count = 0;
	TimePoint start_time = system_clock::now();
	TimePoint end_time;
	double fps = 0;
	logger->info("Starting detection");

	while (true) {
		cap >> frame;
		if (frame.empty()) {
			logger->info("Skipping empty frame");
			continue;
		}
		// get fps
		//std::cout << frame.size();
		if (++frame_count >= 10) {
			end_time = system_clock::now();
			fps = frame_count / ((end_time - start_time)/1.0s);
			start_time = end_time;
			frame_count = 0;
		}
		this->detection_step(&frame);

		// put fps
		cv::putText(frame, std::to_string((int)fps)+" fps", cv::Point(20, 50), 1, 2, (0, 255, 255), 2);
		// put battery_stat
		this->_put_battery_on_frame(&frame);
		
		cv::imshow(cv_window_name, frame);

		char key = (char)cv::waitKey(10);
		if (key == 27 || key == 'q' ||
			cv::getWindowProperty(cv_window_name, cv::WND_PROP_VISIBLE) < 1
			) {
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

		face_detector.visualize(img, face_detection);
		cv::Rect gesture_box = face_detector.generate_bounding_box(face_detection.box, *img, 256, 256);
		cv::rectangle(*img, gesture_box, color, 2);

		cv::Mat gesture_detection_area = (*img)(gesture_box);
		//cv::imshow("Fucked up", gesture_detection_area);
		//return;
		//logger->info(std::to_string(gesture_detection_area));
		Detection gesture_detection = gesture_detector.detect(gesture_detection_area);
		cv::Rect box_scaled(gesture_detection.box);
		box_scaled.x += gesture_box.x;
		box_scaled.y += gesture_box.y;
		cv::rectangle(*img, box_scaled, color, 2);

		if (gesture_detection.score > 0) {
			_last_gesture = system_clock::now();
			stop_tello = false;
			cv::Mat gesture_region = gesture_detection_area(gesture_detection.box);
			ClassifierOutput classified_gesture = gesture_detector.classify(gesture_region);
			buffer.add(classified_gesture.classId);
			gesture_detector.visualize(img, classified_gesture, gesture_box);
		}
	}
}

void Controller::send_command() {
	// accesses _last_face, _last_gesture, stop, buffer, is_landing, tello, vel, debug, stop_tello
	// sets vel, buffer

	while (true) {
		if (!this->stop_tello) {
			if ((system_clock::now() - this->_last_face) > FACE_TIMEOUT ||
				(system_clock::now() - this->_last_gesture) > GESTURE_TIMEOUT) {
				logger->info("No face or gesture: stopping Tello");
				this->stop();
			}
			else {
				vector<int> vel = { 0, 0, 0, -1 };
				Gesture gesture = static_cast<Gesture>(buffer.get());

				if (gesture != NoGesture) {
					logger->debug("Received gesture {}", gesture);
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



void Buffer::add(size_t elem) {
	_buffer.at(elem)++;
}

size_t Buffer::get() {
		auto max_count = std::max_element(_buffer.begin(), _buffer.end());
		if (max_count != _buffer.end() && *max_count >= max_len) {
			size_t val = (std::distance(_buffer.begin(), max_count));
			_buffer.assign(size, default_val);
			return val;
		}
		else {
			return default_val;
		}
}
