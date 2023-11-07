#include "device.h"

using std::vector;
using std::string;

const char Device::STREAM_URL_[] = "udp://0.0.0.0:11111";

void Device::send_rc_control(const velocity_vector_ms_t& vel) {
	logger_->info("rc {} {} {} {}", vel.at(0), vel.at(1), vel.at(2), vel.at(3));
    // TODO
}

void Device::land() {
	logger_->info("land");
    // TODO
}

int Device::get_battery() {
	logger_->info("Battery: {}%", 100);
	return 100; // TODO
}

cv::VideoCapture Device::get_video_stream(int camera_id) {
	cv::VideoCapture cap;
	if (simulate_) {
		cap = cv::VideoCapture(camera_id);
	}
	else {
		cap = cv::VideoCapture(STREAM_URL_, cv::CAP_FFMPEG);
	}
	if (!cap.isOpened()) {
		logger_->error("Unable to get video stream");
		// TODO handle error
	}
	return cap;
}