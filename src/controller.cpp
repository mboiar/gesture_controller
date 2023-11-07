#include <thread>
#include <stdexcept>
#include <atomic>
#include <map>
#include <algorithm>
#include "controller.h"

using std::chrono::system_clock;
using std::chrono::milliseconds;
using std::string;
using std::vector;
using std::atomic;
using TimePoint = std::chrono::time_point<std::chrono::system_clock>;
using namespace std::chrono_literals;


const string Controller::cv_window_name_ = "Device camera";

void Controller::update_battery_stat_() {
	while (true) {
		battery_stat_ = device_->get_battery();
		std::this_thread::sleep_for(WAIT_BATTERY_);
	}
}

void Controller::run(interval_ms_t frame_refresh_rate) {
	std::thread control_thread(&Controller::send_command, this);
	control_thread.detach();
	std::thread battery_thread(&Controller::update_battery_stat_, this);
	battery_thread.detach();

	cv::VideoCapture cap = device_->get_video_stream(0);
	cv::Mat frame;
	cv::namedWindow(cv_window_name_);

	unsigned int frame_count = 0;
	TimePoint start_time = system_clock::now();
	TimePoint end_time;
	double fps = 0;
	logger_->info("Starting detection");

	while (true) {
		cap >> frame;
		if (frame.empty()) {
			logger_->info("Skipping empty frame");
			continue;
		}

		if (++frame_count >= 10) {
			end_time = system_clock::now();
			fps = (double)frame_count / (double)((end_time - start_time)/1.0s);
			start_time = end_time;
			frame_count = 0;
		}
		detect(&frame);

		put_info_on_frame_(&frame, fps);
		
		cv::imshow(cv_window_name_, frame);

		char key = (char)cv::waitKey(frame_refresh_rate);
		if (key == 27 || key == 'q' || (int)key == -29) {
			// TODO clean-up
			break;
		}
	}
	// TODO catch Ctrl+C KeyboardInterrupt (<csignal>?)
}

void Controller::detect(cv::Mat* img) {
	DetectionResult face_detection = face_detector_.detect(*img);
	if (face_detection.score > 0) {
		last_face_ = system_clock::now();
		color_t color = cv::Scalar(0, 0, 255);

		FaceDetector::visualize(img, face_detection);
		bounding_box_t gesture_box = gesture_detector_.get_detection_area(face_detection.box, img->rows, img->cols, 256, 256);
		cv::rectangle(*img, gesture_box, color, 2);

		cv::Mat gesture_detection_region = (*img)(gesture_box);
		//cv::imshow("Gesture detection area", gesture_detection_area);

		ClassifierOutput classified_gesture = gesture_detector_.detect(gesture_detection_region);

		if (classified_gesture.score > 0) {
			last_gesture_ = system_clock::now();
			stop_device_ = false;
			buffer_.add(classified_gesture.class_id);
			gesture_detector_.visualize(img, classified_gesture, gesture_box);
		}
	}
}

void Controller::send_command() {
	while (true) {
		if (!stop_device_) {
			if ((system_clock::now() - last_face_) > FACE_TIMEOUT_ ||
				(system_clock::now() - last_gesture_) > GESTURE_TIMEOUT_) {
				logger_->info("No face or gesture: stopping drone");
				stop();
			}
			else {
				velocity_vector_ms_t velocity = { 0, 0, 0, -1 };
				auto command = static_cast<Command>(buffer_.get());

				if (command != NoGesture) {
					logger_->debug("Received command {}", command);
					if (!is_busy_) {
						switch (command)
						{
						case NoGesture:
							break;
						case Stop:
							stop();
							break;
						case Left:
							velocity[0] = -1*speed_increment_[0];
							velocity[3] = 0;
							break;
						case Right:
							velocity[0] = speed_increment_[0];
							velocity[3] = 0;
							break;
						case Up:
							velocity[2] = speed_increment_[2];
							velocity[3] = 0;
							break;
						case Down:
							velocity[2] = -1*speed_increment_[2];
							velocity[3] = 0;
							break;
						case Forward:
							velocity[1] = speed_increment_[1];
							velocity[3] = 0;
							break;
						case Back:
							velocity[1] = -1*speed_increment_[1];
							velocity[3] = 0;
							break;
						case Land:
							device_->land();
							is_busy_ = true;
							break;
						default:
							break;
						}
					}

					if (velocity[3] != -1 && velocity_ != velocity) {
						velocity_ = velocity;
						if (!dry_run_) {
							device_->send_rc_control(velocity);
						}
					}
				}
			}
		}
		std::this_thread::sleep_for(WAIT_RC_CONTROL_);
	}
}

void Controller::stop() {
	velocity_ = { 0, 0, 0, 0 };
	stop_device_ = true;
	device_->send_rc_control(velocity_);
}

void Controller::put_info_on_frame_(cv::Mat* frame, double fps/*, TODO bool verbose*/) {
	string battery_text("No battery info");
	if (battery_stat_ > 0) {
		battery_text = std::to_string(battery_stat_) + "%";
	}
	cv::putText(*frame, battery_text, cv::Point(20, 100), 1, 2, cv::Scalar(0, 255, 255), 2);

    cv::putText(*frame, std::to_string((int)fps)+" fps", cv::Point(20, 50), 1, 2, cv::Scalar(0, 255, 255), 2);
}


void Buffer::add(class_id_t class_id) {
	buffer_.at(class_id)++;
}

class_id_t Buffer::get() {
		auto curr_max_count_it = std::max_element(buffer_.begin(), buffer_.end());
		if (curr_max_count_it != buffer_.end() && *curr_max_count_it >= max_count_) {
			class_id_t class_id = std::distance(buffer_.begin(), curr_max_count_it);
			buffer_.assign(size_, 0);
			return class_id;
		}
		else {
			return default_class_id_;
		}
}
