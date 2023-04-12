#include "drone.h"

using std::vector;
using std::string;

const char Drone::DRONE_STREAM_URL[] = "udp://0.0.0.0:11111";

void Drone::send_rc_control(const vector<int>& vel) {
	logger->info("rc {} {} {} {}", vel.at(0), vel.at(1), vel.at(2), vel.at(3));
	//for (auto vel_i : vel) {
	//	std::cout << vel_i << " ";
	//}
	//std::cout << std::endl;
}

void Drone::land() {
	logger->info("land");
}

int Drone::get_battery() {
	logger->info("Battery: {}%", 100);
	return 100;
}

cv::VideoCapture Drone::get_video_stream() {
	cv::VideoCapture cap;
	if (simulate) {
		cap = cv::VideoCapture(0);
	}
	else {
		cap = cv::VideoCapture(DRONE_STREAM_URL, cv::CAP_FFMPEG);
	}
	if (!cap.isOpened()) {
		logger->error("Unable to get video stream");
		// TODO handle error
	}
	return cap;
}