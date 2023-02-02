#include "gesture_detection.h"

using cv::dnn::Net;

GestureDetector::GestureDetector() {
    // TODO setup detector
}

Detection GestureDetector::detect(const cv::Mat& img) {
    // TODO implement detection
}

void GestureDetector::visualize(cv::Mat* img, const Detection& detection, const Gesture& gesture) {
    // TODO implement visualization of detection results
}
