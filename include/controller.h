/**
 * @file controller.hpp
 *
 * @brief Device controller class and its components.
 *
 * @author Maks Boiar
 *
 */

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
#include "device.h"

using std::string;
using std::vector;
using std::chrono::milliseconds;
using cv::Mat;
using std::atomic;
using TimePoint = std::chrono::time_point<std::chrono::system_clock>;
using AsyncLogger = std::shared_ptr<spdlog::logger>;

using interval_ms_t = unsigned long;
using class_id_t = unsigned long;

class Buffer {
	// TODO refactor
	vector<unsigned int> buffer_;
	unsigned int max_count_;
    size_t size_;
    class_id_t default_class_id_;
public:
    /**
     * A constructor.
     * @param max_count maximum count for a class_id before buffer is flushed
     * @param size_ number of classes
     * @param default_class_id to be returned when queried buffer is not full
     */
	Buffer(unsigned int max_count, size_t size_, class_id_t default_class_id = 0) : max_count_(max_count), size_(size_), default_class_id_(default_class_id) {
        buffer_ = vector<unsigned int>(size_);
	}
	void add(class_id_t class_id);
	class_id_t get();
    [[nodiscard]] size_t size() const { return buffer_.size(); }
    unsigned int operator[](class_id_t class_id) const { return buffer_[class_id]; }
};

/**
 * Device controller based on gesture recognition.
 */
class Controller {
	constexpr static size_t buffer_len_ = 5;
	constexpr static int speed_increment_[3] = { 10, 10, 10 };
	constexpr static milliseconds WAIT_RC_CONTROL_ = milliseconds(500);
	constexpr static milliseconds WAIT_BATTERY_ = milliseconds(4000);
	constexpr static milliseconds FACE_TIMEOUT_ = milliseconds(1000);
	constexpr static milliseconds GESTURE_TIMEOUT_ = milliseconds(1000);

	Device *device_;
	bool dry_run_;
	FaceDetector face_detector_;
	GestureDetector gesture_detector_;
	Buffer buffer_;
	atomic<int> battery_stat_ = -1;
	TimePoint last_gesture_ = TimePoint();
	TimePoint last_face_ = TimePoint();
	bool stop_device_ = false;
	bool is_busy_ = false;
	velocity_vector_ms_t velocity_ = { 0,0,0,0 };
	static const string cv_window_name_;
	AsyncLogger logger_;
    string name_;

    /**
     * put additional information on the video frame.
     * @param frame
     * @param fps frames per second speed
     */
	void put_info_on_frame_(Mat * frame, double fps/*, bool verbose = true*/);

    /**
     * continuously query device's battery status and save value in `battery_stat` attribute.
     */
    void update_battery_stat_();

    /**
     * Send a command from the buffer to the connected device.
     */
    void send_command();

public:
    /**
     * A constructor.
     * @param device pointer to a `Device` instance to be controlled.
     * @param dry_run if true, commands are not being sent to the actual device.
     * @param name controller instance name
     */
	Controller(
            Device* device, bool dry_run, const string& face_detector_path, const string& gesture_detector_path,
            const string& name="CONTROLLER"
            ) :
		device_(device),
		dry_run_(dry_run),
		face_detector_(face_detector_path),
		gesture_detector_(gesture_detector_path),
		buffer_(buffer_len_, GestureCount),
        name_(name) {
		logger_ = spdlog::get(name_);
		if (!logger_) {
			logger_ = spdlog::stdout_color_mt(name_);
		}
		logger_->set_level(spdlog::level::info);
	};

    /**
     * run inference on input stream and control device.
     * @param frame_refresh_rate indicates how often to refresh application window (in ms).
     */
	void run(interval_ms_t frame_refresh_rate = 25);

    /**
     * Detect gesture in an input image.
     * @param image a matrix containing an image where gesture will be detected.
     */
	void detect(image_t* image);

    /**
     * Stop control and try to land the device.
     */
	void stop();

};

#endif