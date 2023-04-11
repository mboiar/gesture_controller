#ifndef TELLO_H
#define TELLO_H

#include <vector>
#include <string>
#include <chrono>
#include <atomic>

#include <opencv2\opencv.hpp>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

using std::string;
using std::vector;
using AsyncLogger = std::shared_ptr<spdlog::logger>;


class Tello {
	AsyncLogger logger;
	static const char TELLO_STREAM_URL[];
	bool simulate;

public:
	int get_battery();
	void send_rc_control(const vector<int>& vel);
	void land();
	cv::VideoCapture get_video_stream();
	Tello(bool sim = true) : simulate(sim) {
		string name("TELLO");
		logger = spdlog::get(name);
		if (!logger) {
			logger = spdlog::stdout_color_mt(name);
		}
		logger->set_level(spdlog::level::info);
	}
};

#endif