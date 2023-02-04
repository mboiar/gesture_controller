#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <vector>
#include <string>
#include <chrono>
#include <atomic>

#include "face_detection.h"
#include "gesture_detection.h"

using std::string;
using std::vector;
using std::chrono::milliseconds;
using cv::Mat;
using std::atomic;
using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

class Tello {
	spdlog::logger logger;
	static const char TELLO_STREAM_URL[];
	bool simulate;

public:
	int get_battery();
	void send_rc_control(const vector<int>& vel);
	void land();
	cv::VideoCapture get_video_stream();
	Tello(bool sim = true) : logger("TELLO"), simulate(sim) {
		logger.set_level(spdlog::level::info);
	}
};

template <class T>
class Buffer {
	// TODO thread-safe buffer
	vector<int> _buffer;
	int max_len, size;
public:
	unsigned int buffer_len;
	Buffer(unsigned int bl, unsigned int size) : max_len(bl), size(size) {
		_buffer = vector<int>(size);
	}
	void add(const T &elem);
	T get();
};

class Controller {
	constexpr static int buffer_len = 5;
	constexpr static int dw[3] = { 10, 10, 10 };
	constexpr static long WAIT_RC_CONTROL = 500;
	constexpr static long WAIT_BATTERY = 4000;
	constexpr static milliseconds FACE_TIMEOUT = milliseconds(1000);
	constexpr static milliseconds GESTURE_TIMEOUT = milliseconds(1000);
	Tello *tello;
	bool debug;
	Buffer<Gesture> buffer;
	FaceDetector face_detector;
	GestureDetector gesture_detector;
	atomic<int> battery_stat = -1;
	TimePoint _last_gesture = TimePoint();
	TimePoint _last_face = TimePoint();
	bool stop_tello = false;
	bool is_landing = false;
	vector<int> vel = { 0 };
	static const string cv_window_name;
	spdlog::logger logger;
	void _put_battery_on_frame(Mat *);
public:
	Controller(Tello* tello, bool debug) :
		tello(tello),
		debug(debug),
		face_detector(),
		gesture_detector(),
		logger("CONTROLLER"),
		buffer(buffer_len, GestureCount) {
		logger.set_level(spdlog::level::info);
	};

	void run();
	void detection_step(Mat*);
	void send_command();
	void get_battery_stat();
	void stop();
	//cv::Rect get_gesture_bounds(const cv::Rect&, const cv::Mat&, int w, int h);
};

#endif