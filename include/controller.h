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

//using Clock = std::chrono::high_resolution_clock;
//using TimePoint = std::chrono::time_point<chrono::system_clock>;

class Tello {
public:
	int get_battery() { return 50; };
	void send_rc_control(const vector<int>& vel);
	void land();
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
	Tello tello;
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
	void _put_battery_on_frame(Mat *);
public:
	// TODO add logging
	Controller(Tello& tello, bool debug) :
		tello(tello),
		debug(debug),
		face_detector(),
		gesture_detector(),
		buffer(buffer_len, GestureCount) {};

	void run();
	void detection_step(Mat*);
	void send_command();
	void get_battery_stat();
	void stop();
	//cv::Rect get_gesture_bounds(const cv::Rect&, const cv::Mat&, int w, int h);
};

#endif