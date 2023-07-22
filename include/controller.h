#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <atomic>

#include <opencv2/opencv.hpp>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include "face_detection.h"
#include "gesture_detection.h"
#include "drone.h"

using std::string;
using std::vector;
using std::chrono::milliseconds;
using cv::Mat;
using std::atomic;
using TimePoint = std::chrono::time_point<std::chrono::system_clock>;
using AsyncLogger = std::shared_ptr<spdlog::logger>;


class Buffer {
	// TODO thread-safe buffer
	vector<int> _buffer;
	int max_len;
	size_t size, default_val;
public:
	unsigned int buffer_len;
	Buffer(unsigned int bl_, size_t size_, size_t default_val_ = 0) : max_len(bl_), size(size_), default_val(default_val_) {
		_buffer = vector<int>(size);
	}
	void add(size_t elem);
	size_t get();
};

class Controller {
	constexpr static int buffer_len = 5;
	constexpr static int dw[3] = { 10, 10, 10 };
	constexpr static long WAIT_RC_CONTROL = 500;
	constexpr static long WAIT_BATTERY = 4000;
	constexpr static milliseconds FACE_TIMEOUT = milliseconds(1000);
	constexpr static milliseconds GESTURE_TIMEOUT = milliseconds(1000);
	Drone *drone;
	bool debug;
	FaceDetector face_detector;
	GestureDetector gesture_detector;
	Buffer buffer;
	atomic<int> battery_stat = -1;
	TimePoint _last_gesture = TimePoint();
	TimePoint _last_face = TimePoint();
	bool stop_drone = false;
	bool is_landing = false;
	vector<int> vel = { 0 };
	static const string cv_window_name;
	AsyncLogger logger;
	void _put_battery_on_frame(Mat *);
public:
	Controller(Drone* drone, bool debug) :
		drone(drone),
		debug(debug),
		face_detector(),
		gesture_detector(),
		buffer(buffer_len, GestureCount) {
		string name("CONTROLLER");
		logger = spdlog::get(name);
		if (!logger) {
			logger = spdlog::stdout_color_mt(name);
		}
		logger->set_level(spdlog::level::info);
	};

	void run(int frame_refresh_rate = 25);
	void detection_step(Mat*);
	void send_command();
	void get_battery_stat();
	void stop();
	//cv::Rect get_gesture_bounds(const cv::Rect&, const cv::Mat&, int w, int h);
};

#endif