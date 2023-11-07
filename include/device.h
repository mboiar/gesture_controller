/**
 * @file device.h
 *
 * @brief Generic controllable device interface.
 *
 * @author Maks Boiar
 *
 */

#ifndef DEVICE_H
#define DEVICE_H

#include <vector>
#include <string>
#include <chrono>
#include <atomic>

#include <opencv2/opencv.hpp>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

using std::string;
using std::vector;
using AsyncLogger = std::shared_ptr<spdlog::logger>;

using velocity_vector_ms_t = vector<int>;

/**
 * List of command names for device control.
 */
enum Command {
    NoGesture = 0,
    Left,
    Right,
    Up,
    Down,
    Forward,
    Back,
    Stop,
    Land,
    GestureCount
};

/**
 * Abstract controllable device with a camera.
 */
class Device {
	AsyncLogger logger_;
	bool simulate_;
    static const char STREAM_URL_[];
public:
    explicit Device(bool simulate = true) : simulate_(simulate) {
        string name("DEVICE");
        logger_ = spdlog::get(name);
        if (!logger_) {
            logger_ = spdlog::stdout_color_mt(name);
        }
        logger_->set_level(spdlog::level::info);
    }
	int get_battery();
	void send_rc_control(const velocity_vector_ms_t& velocity);
	void land();

    /**
     * Capture video stream.
     *
     * @param camera_id id of the camera whose stream will be captured
     */
	cv::VideoCapture get_video_stream(int camera_id);

    /**
     * Enable video streaming.
     */
	void streamon(){};

    /**
     * Connect to a device.
     */
	void connect(){};
};

#endif