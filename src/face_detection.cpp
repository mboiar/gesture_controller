#include "face_detection.h"

using cv::dnn::Net;

FaceDetector::FaceDetector() {
    // TODO setup detector
}

Detection FaceDetector::detect(const cv::Mat &img) {
    // TODO implement detection
    return Detection();
}

void FaceDetector::visualize(cv::Mat* img, const Detection& detection) {
    // TODO implement visualization of detection results
}
