#include "gesture_detection.h"
#include <stdexcept>

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

GestureDetector::GestureDetector(const string &detector_path) {
    string name("DETECTION");
    logger_ = spdlog::get(name);
    if (!logger_) {
        logger_ = spdlog::stdout_color_mt(name);
    }
    logger_->set_level(spdlog::level::debug);

    detector_ = cv::dnn::readNet(detector_path);
}

ClassifierOutput GestureDetector::detect(const image_t& img) {
    cv::Mat blob = preprocess_image(img);
    detector_.setInput(blob);
    std::vector<std::string> outNames = detector_.getUnconnectedOutLayersNames();
    std::vector<cv::Mat> outs;
    detector_.forward(outs, outNames);
    for (auto i : outs) {
        softmax(i, i);
    }
    cv::Point classIdPoint_leading_hand;
    score_t confidence_leading_hand;
    cv::Point classIdPoint_gesture;
    score_t confidence_gesture;
    cv::minMaxLoc(outs.at(1).reshape(1, 1), nullptr, &confidence_gesture, nullptr, &classIdPoint_gesture);
    logger_->info("Gesture class: {} conf: {:.2f}", classIdPoint_gesture.x, confidence_gesture);
    cv::minMaxLoc(outs.at(0).reshape(1, 1), nullptr, &confidence_leading_hand, nullptr, &classIdPoint_leading_hand);
    logger_->info("Leading hand: {} conf: {:.2f}", classIdPoint_leading_hand.x, confidence_leading_hand);

    ClassifierOutput classified_gesture = ClassifierOutput(confidence_gesture, classIdPoint_gesture.x);
    return classified_gesture;
}

cv::Mat GestureDetector::preprocess_image(const image_t& img) {
    // rescale frame to [0, 1] than resize and pad to [224, 224, 3]
    double scale = 1.0 / 255.0;
    int inpWidth = 224;
    int inpHeight = 224;
    image_t resized_img = img;
    resize_and_pad(resized_img, resized_img, cv::Size(inpWidth, inpHeight));
    return cv::dnn::blobFromImage(resized_img, scale, cv::Size(inpWidth, inpHeight), 0, true, false);
}

void GestureDetector::visualize(image_t* img, const ClassifierOutput& classified_gesture, const bounding_box_t& gesture_box, const color_t& color) {
    // draw gesture name
    cv::putText(*img, gesture_map[classified_gesture.class_id], cv::Point(gesture_box.x + 80, gesture_box.y - 20), 0, 0.5, color, 1);
    // draw landmarks
    if (!classified_gesture.landmarks.empty()) {
        // TODO draw gesture landmarks
    }
    // draw score
    cv::putText(*img, std::to_string((int)(classified_gesture.score * 100)) + " %", cv::Point(gesture_box.x, gesture_box.y - 20), 0, 0.5, color, 1);
}

/**
 * Given a face detection box, define an area of the image where gestures will be detected..
 * @param box area of the image where face has been detected in (x1, y1, x2, y2) format
 * @param img_width image width
 * @param img_height image height
 * @param w new detection area width
 * @param h new detection area height
 * @return bounding box of an area where gestures will be detected
 */
bounding_box_t GestureDetector::get_detection_area(const bounding_box_t& face_box, int img_width, int img_height, int w, int h) {
    int x1 = face_box.x + face_box.width;
    int y1 = face_box.y;
    int x2 = std::min(x1 + w, img_width);
    int y2 = std::min(y1 + h, img_height);
    return {cv::Point(x1, y1), cv::Point(x2, y2)};
}
