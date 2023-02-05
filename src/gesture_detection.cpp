#include "gesture_detection.h"

using cv::dnn::Net;
using std::string;

std::map<int, std::string> gesture_map = {
    { 1, "Left"},
    { 2, "Right"},
    { 3, "Up"},
    { 4, "Down"},
    { 5, "Forward"},
    { 6, "Back"},
    { 7, "Stop"},
    { 8, "Land"},
};

GestureDetector::GestureDetector() {
    string name("DETECTION");
    logger = spdlog::get(name);
    if (!logger) {
        logger = spdlog::stdout_color_mt(name);
    }
    logger->set_level(spdlog::level::info);
}

GestureDetection GestureDetector::detect(const cv::Mat& img) {
	// TODO implement detection
    GestureDetection detection = GestureDetection(0.99, Left);
    logger->debug("Gesture {}: {:03.1f}%", detection.gesture, detection.score*100);
	return detection;
}

void GestureDetector::visualize(cv::Mat* img, const GestureDetection& detection, const cv::Rect& gesture_box) {
    cv::Scalar color = cv::Scalar(0, 255, 255);
    // draw gesture
    cv::putText(*img, gesture_map[detection.gesture], cv::Point(gesture_box.x + 80, gesture_box.y - 20), 0, 0.5, color, 1);

    // draw score
    cv::putText(*img, std::to_string((int)(detection.score * 100)) + " %", cv::Point(gesture_box.x, gesture_box.y - 20), 0, 0.5, color, 1);

}
