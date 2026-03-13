/**
 * @file device.h
 *
 * @brief Generic controllable device interface.
 *
 * @author Maks Boiar
 *
 */

#pragma once

#include <atomic>
#include <chrono>
#include <string>
#include <vector>

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include <opencv2/opencv.hpp>

using std::string;
using std::vector;
using AsyncLogger = std::shared_ptr<spdlog::logger>;

using velocity_vector_ms_t = vector<int>;

/**
 * Abstract controllable device with a camera.
 */
class GenericDevice {
public:
  GenericDevice(bool simulate = true) : simulate_(simulate) {
    string name("DEVICE");
    logger_ = spdlog::get(name);
    if (!logger_) {
      logger_ = spdlog::stdout_color_mt(name);
    }
  }

  void send_rc_control(const velocity_vector_ms_t &velocity);

  virtual void stop();

  //   /**
  //    * Capture video stream.
  //    *
  //    * @param camera_id id of the camera whose stream will be captured
  //    */
  cv::VideoCapture get_video_stream(int camera_id);

  //   /**
  //    * Connect to a device.
  //    */
  virtual int connect(const std::string &server_name);

  void get_frame(cv::Mat *frame) { cap >> *frame; }

  virtual bool get_status() const;

  virtual std::string get_mode() const;

  virtual std::vector<double> get_position();

  ~GenericDevice() = default;

protected:
  AsyncLogger logger_;
  bool simulate_;
  cv::VideoCapture cap;
  std::string STREAM_URL_;
};
