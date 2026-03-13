#include "device.hpp"

using std::string;
using std::vector;

cv::VideoCapture GenericDevice::get_video_stream(int camera_id) {

  if (simulate_) {
    logger_->info("Opening stream {}", camera_id);
    // while (true) {
    //	if (cap.open(camera_id++)) {
    //		logger_->info("Opened {}", camera_id);
    //	}
    // }
    cap = cv::VideoCapture(camera_id, cv::CAP_MSMF);
    logger_->info(cap.getBackendName());
  } else {
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