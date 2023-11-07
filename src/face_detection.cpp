#include "face_detection.h"

#include <stdexcept>
#include <algorithm>

using cv::dnn::Net;
using std::chrono::system_clock;
using TimePoint = std::chrono::time_point<std::chrono::system_clock>;


FaceDetector::FaceDetector(const std::string& detector_path) {
    string name("DETECTION");
    logger_ = spdlog::get(name);
    if (!logger_) {
        logger_ = spdlog::stdout_color_mt(name);
    }
    logger_->set_level(spdlog::level::info);

    detector_ = cv::CascadeClassifier();
    string detector_name = cv::samples::findFileOrKeep(detector_path);
    logger_->info("Load face cascade classifier");
    if (!detector_.load(detector_name)) {
        logger_->error("Error loading classifier");
        // TODO handle error
    }
}


DetectionResult FaceDetector::detect(const image_t &image) {
    vector<bounding_box_t> faces;
    image_t gray;
    cv::cvtColor(image, gray, cv::COLOR_RGBA2GRAY, 0);

    cv::resize(gray, gray, cv::Size(), 1 / scale_, 1 / scale_, cv::INTER_LINEAR);
    cv::equalizeHist(gray, gray);
    detector_.detectMultiScale(gray, faces, 1.1, 3, 0);

    if (faces.empty()){
        return {};
    }
    bounding_box_t max_face = *std::max_element(faces.begin(), faces.end(), [](cv::Rect face1, cv::Rect face2){return face1.area() < face2.area();});

    // TODO get detection score
    score_t score = 0.99;
    DetectionResult detection = DetectionResult(max_face, score);
    rescale_box(detection.box, detection.box, scale_);

    logger_->debug("Detected face: {:03.1f}% at ({} {} {} {})", score*100, max_face.x, max_face.y, max_face.x+max_face.width, max_face.y+max_face.height);

    return detection;
}


void FaceDetector::visualize(image_t* img, const DetectionResult& detection, const color_t& color) {
    cv::rectangle(*img, detection.box, color, 1);
    cv::putText(*img, std::to_string((int)(detection.score*100))+" %", cv::Point(detection.box.x, detection.box.y - 20), 0, 0.5, color, 1);
}
