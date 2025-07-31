#include "device.h"

using std::vector;
using std::string;
//
const char Device::STREAM_URL_[] = "udp://0.0.0.0:11111";

void Device::send_rc_control(const velocity_vector_ms_t& vel) {
	logger_->info("rc {} {} {} {}", vel.at(0), vel.at(1), vel.at(2), vel.at(3));
}

void Device::land() {
	logger_->info("land");
    // TODO
}

int Device::get_battery() {
	//logger_->info("Battery: {}%", 100);
	return 100; // TODO
}

cv::VideoCapture Device::get_video_stream(int camera_id) {

	if (simulate_) {
		logger_->info("Opening stream {}", camera_id);
		//while (true) {
		//	if (cap.open(camera_id++)) {
		//		logger_->info("Opened {}", camera_id);
		//	}
		//}
		cap = cv::VideoCapture(camera_id, cv::CAP_MSMF);
		logger_->info(cap.getBackendName());
	}
	else {
		logger_->info("Opening stream... {}", STREAM_URL_);
		cap = cv::VideoCapture(STREAM_URL_, cv::CAP_FFMPEG);
	}
	if (!cap.isOpened()) {
		logger_->error("Unable to get video stream");
		// TODO handle error
	}
	logger_->info("Done");
	return cap;
}