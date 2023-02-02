#include "gesture_detection.h"

using cv::dnn::Net;

GestureDetector::GestureDetector() {
	// TODO setup detector
}

GestureDetection GestureDetector::detect(const cv::Mat& img) {
	// TODO implement detection
	return GestureDetection();
}

void GestureDetector::visualize(cv::Mat* img, const GestureDetection& detection) {
	// TODO implement visualization of detection results
}
