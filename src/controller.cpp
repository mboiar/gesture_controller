#include <thread>
#include <stdexcept>
#include <atomic>
#include <map>
#include <algorithm>
#include "controller.h"

extern "C" {
#include <open62541/client_config_default.h>
#include <open62541/client_highlevel.h>

}

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
		servoOk = device_->get_opc_value<bool>("::AsGlobalPV:gMainInterface.Robot.Status.ServoOK");
		RobotPos[0].store(device_->get_opc_value<double>("::AsGlobalPV:MpDelta4Axis_0.X"));
		RobotPos[1].store(device_->get_opc_value<double>("::AsGlobalPV:MpDelta4Axis_0.Y"));
		RobotPos[2].store(device_->get_opc_value<double>("::AsGlobalPV:MpDelta4Axis_0.Z"));
		UA_String uastr = device_->get_opc_value<UA_String>("::AsGlobalPV:gModeText");
		char* modeTextVal = (char*)malloc(uastr.length + 1);
		memcpy(modeTextVal, uastr.data, uastr.length);
		modeTextVal[uastr.length] = '\0';
		//logger_->info("Read {}", modeTextVal);
		//std::lock_guard<std::mutex> lock(modeMutex);
		modeText = modeTextVal;
		free(modeTextVal);
		//std::lock_guard<std::mutex> unlock(modeMutex);
		std::this_thread::sleep_for(WAIT_BATTERY_);
	}
}

void Controller::run(interval_ms_t frame_refresh_rate) {
	std::thread control_thread(&Controller::send_command, this);
	control_thread.detach();
	std::thread battery_thread(&Controller::update_battery_stat_, this);
	battery_thread.detach();

	cv::Mat frame;
	cv::namedWindow(cv_window_name_);

	unsigned int frame_count = 0;
	TimePoint start_time = system_clock::now();
	TimePoint end_time;
	double fps = 0;
	logger_->info("Starting detection");

	while (true) {
		device_->get_frame(&frame);
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
		if (modeText != "AUTO") {
			try {
				detect(&frame);
			}
			catch (const std::exception& e) {
				std::cerr << "[DETECT] Caught exception: " << e.what() << std::endl;
			}
		}
		//std::cout << "Detection successful" << std::endl;
		try {
			put_info_on_frame_(&frame, fps);
		}
		catch (const std::exception& e) {
			std::cerr << "[PUT INFO] Caught exception: " << e.what() << std::endl;
		}
		//std::cout << "Visualization successful" << std::endl;

		try {
			cv::imshow(cv_window_name_, frame);
		}
		catch (const std::exception& e) {
			std::cerr << "[IMSHOW] Caught exception: " << e.what() << std::endl;
		}

		char key = (char)cv::waitKey(frame_refresh_rate);
		if (key == 27 || key == 'q' || (int)key == -29) {
			// TODO clean-up
			break;
		}
	}
	// TODO catch Ctrl+C KeyboardInterrupt (<csignal>?)
}

void Controller::detect(cv::Mat* img) {
	//std::cout << "0 ";
	DetectionResult face_detection = face_detector_.detect(*img);
	//std::cout << "1 ";
	if (face_detection.score > 0) {
		last_face_ = system_clock::now();
		color_t color = cv::Scalar(0, 0, 255);

		FaceDetector::visualize(img, face_detection);
		bounding_box_t gesture_box = gesture_detector_.get_detection_area(face_detection.box, img->cols, img->rows, 256, 256);
		cv::rectangle(*img, gesture_box, color, 2);

		cv::Mat gesture_detection_region = (*img)(gesture_box);
		//cv::imshow("Gesture detection area", gesture_detection_area);

		ClassifierOutput classified_gesture = gesture_detector_.detect(gesture_detection_region);

		//std::cout << "3 ";

		if (classified_gesture.score > 0 && classified_gesture.class_id != 18) {
			last_gesture_ = system_clock::now();
			stop_device_ = false;
			buffer_.add(classified_gesture.class_id);
			//std::cout << "4 ";
			gesture_detector_.visualize(img, classified_gesture, gesture_box);
			//std::cout << "5 " << std::endl;
		}
	}
}

void Controller::send_command() {
	while (true) {
		if (!stop_device_) {
			if ((system_clock::now() - last_face_) > FACE_TIMEOUT_ ||
				(system_clock::now() - last_gesture_) > GESTURE_TIMEOUT_) {
				logger_->info("No face or gesture: stopping");
				stop();
			}
			else {
				velocity_vector_ms_t velocity = { 0, 0, 0, -1 };
				auto command = static_cast<Command>(buffer_.get());

				if (command != NoGesture) {
					logger_->info("Received command {}", static_cast<int>(command));
					if (!is_busy_) {
						switch (command)
						{
						case NoGesture:
							device_->set_opc_value<bool>("::Manual:JogYUp", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
							device_->set_opc_value<bool>("::Manual:JogYDown", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
							device_->set_opc_value<bool>("::Manual:JogXUp", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
							device_->set_opc_value<bool>("::Manual:JogXDown", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
							device_->set_opc_value<bool>("::Manual:JogZUp", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
							device_->set_opc_value<bool>("::Manual:JogZDown", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
							break;
						case Stop:
							device_->set_opc_value<bool>("::Manual:JogYUp", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
							device_->set_opc_value<bool>("::Manual:JogYDown", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
							device_->set_opc_value<bool>("::Manual:JogXUp", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
							device_->set_opc_value<bool>("::Manual:JogXDown", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
							device_->set_opc_value<bool>("::Manual:JogZUp", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
							device_->set_opc_value<bool>("::Manual:JogZDown", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
							stop();
							break;
						case JogYUp:
							velocity[0] = -1*speed_increment_[0];
							velocity[3] = 0;
							device_->set_opc_value<bool>("::Manual:JogYUp", true, &UA_TYPES[UA_TYPES_BOOLEAN]);
							device_->set_opc_value<bool>("::Manual:JogYDown", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
							device_->set_opc_value<bool>("::Manual:JogXUp", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
							device_->set_opc_value<bool>("::Manual:JogXDown", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
							device_->set_opc_value<bool>("::Manual:JogZUp", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
							device_->set_opc_value<bool>("::Manual:JogZDown", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
							break;
						case JogYDown:
							velocity[0] = speed_increment_[0];
							velocity[3] = 0;
							device_->set_opc_value<bool>("::Manual:JogYDown", true, &UA_TYPES[UA_TYPES_BOOLEAN]);
							device_->set_opc_value<bool>("::Manual:JogYUp", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
							device_->set_opc_value<bool>("::Manual:JogXUp", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
							device_->set_opc_value<bool>("::Manual:JogXDown", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
							device_->set_opc_value<bool>("::Manual:JogZUp", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
							device_->set_opc_value<bool>("::Manual:JogZDown", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
							break;
						case JogXUp:
							velocity[2] = speed_increment_[2];
							velocity[3] = 0;
							device_->set_opc_value<bool>("::Manual:JogXUp", true, &UA_TYPES[UA_TYPES_BOOLEAN]);
							device_->set_opc_value<bool>("::Manual:JogYUp", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
							device_->set_opc_value<bool>("::Manual:JogYDown", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
							device_->set_opc_value<bool>("::Manual:JogXDown", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
							device_->set_opc_value<bool>("::Manual:JogZUp", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
							device_->set_opc_value<bool>("::Manual:JogZDown", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
							break;
						case JogXDown:
							velocity[2] = -1*speed_increment_[2];
							velocity[3] = 0;
							device_->set_opc_value<bool>("::Manual:JogXDown", true, &UA_TYPES[UA_TYPES_BOOLEAN]);
							device_->set_opc_value<bool>("::Manual:JogYUp", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
							device_->set_opc_value<bool>("::Manual:JogYDown", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
							device_->set_opc_value<bool>("::Manual:JogXUp", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
							device_->set_opc_value<bool>("::Manual:JogZUp", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
							device_->set_opc_value<bool>("::Manual:JogZDown", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
							break;
						case JogZUp:
							velocity[1] = speed_increment_[1];
							velocity[3] = 0;
							device_->set_opc_value<bool>("::Manual:JogZUp", true, &UA_TYPES[UA_TYPES_BOOLEAN]);
							device_->set_opc_value<bool>("::Manual:JogYUp", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
							device_->set_opc_value<bool>("::Manual:JogYDown", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
							device_->set_opc_value<bool>("::Manual:JogXUp", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
							device_->set_opc_value<bool>("::Manual:JogXDown", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
							device_->set_opc_value<bool>("::Manual:JogZDown", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
							break;
						case JogZDown:
							velocity[1] = -1*speed_increment_[1];
							velocity[3] = 0;
							device_->set_opc_value<bool>("::Manual:JogZDown", true, &UA_TYPES[UA_TYPES_BOOLEAN]);
							device_->set_opc_value<bool>("::Manual:JogYUp", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
							device_->set_opc_value<bool>("::Manual:JogYDown", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
							device_->set_opc_value<bool>("::Manual:JogXUp", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
							device_->set_opc_value<bool>("::Manual:JogXDown", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
							device_->set_opc_value<bool>("::Manual:JogZUp", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
							break;
						case ToolOn:
							device_->land();
							//is_busy_ = true;
							break;
						case ToolOff:
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
	device_->set_opc_value<bool>("::Manual:JogYUp", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
	device_->set_opc_value<bool>("::Manual:JogYDown", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
	device_->set_opc_value<bool>("::Manual:JogXUp", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
	device_->set_opc_value<bool>("::Manual:JogXDown", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
	device_->set_opc_value<bool>("::Manual:JogZUp", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
	device_->set_opc_value<bool>("::Manual:JogZDown", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
	velocity_ = { 0, 0, 0, 0 };
	stop_device_ = true;
	device_->send_rc_control(velocity_);
}

void Controller::put_info_on_frame_(cv::Mat* frame, double fps/*, TODO bool verbose*/) {
	cv::putText(*frame, "ServoOK: "+std::to_string(servoOk), cv::Point(20, 100), 1, 1, cv::Scalar(0, 0, 0), 2);
	//std::lock_guard<std::mutex> lock(modeMutex);
	cv::putText(*frame, "Mode: " + modeText, cv::Point(20, 120), 1, 1, cv::Scalar(0, 0, 0), 2);
	cv::putText(*frame, "PosX: " + std::to_string(RobotPos[0].load()), cv::Point(20, 200), 1, 1, cv::Scalar(0, 0, 0), 2);
	cv::putText(*frame, "PosY: " + std::to_string(RobotPos[1].load()), cv::Point(20, 220), 1, 1, cv::Scalar(0, 0, 0), 2);
	cv::putText(*frame, "PosZ: " + std::to_string(RobotPos[2].load()), cv::Point(20, 240), 1, 1, cv::Scalar(0, 0, 0), 2);
    cv::putText(*frame, std::to_string((int)fps)+" fps", cv::Point(20, 80), 1, 2, cv::Scalar(0, 0, 0), 2);
}


void Buffer::add(class_id_t class_id) {
	//std::cout << buffer_.size() << " " << class_id << std::endl;
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
