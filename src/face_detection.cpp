#include "face_detection.h"

#include <stdexcept>

using cv::dnn::Net;
using std::chrono::system_clock;
using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

FaceDetector::FaceDetector() {
    string name("DETECTION");
    logger = spdlog::get(name);
    if (!logger) {
        logger = spdlog::stdout_color_mt(name);
    }
    logger->set_level(spdlog::level::info);

    classifier = cv::CascadeClassifier();
    //string classifier_path("data/cascade/lbpcascade_frontalface_improved.xml");
    string classifier_path("data/cascade/haarcascade_frontalface_default.xml");
    string classifier_name = cv::samples::findFileOrKeep(classifier_path);
    logger->info("Load face cascade classifier");
    if (!classifier.load(classifier_name)) {
        logger->error("Error loading classifier");
        // TODO handle error
    }
}

Detection FaceDetector::detectAndDisplay(const cv::Mat &img) {
    vector<cv::Rect> faces;
    cv::Mat gray;
    cv::cvtColor(img, gray, cv::COLOR_RGBA2GRAY, 0);

    cv::resize(gray, gray, cv::Size(), 1 / scale, 1 / scale, cv::INTER_LINEAR);
    cv::equalizeHist(gray, gray);
    classifier.detectMultiScale(gray, faces, 1.1, 3, 0);

    //cv::Rect face = faces.at(0);
    if (faces.empty()){
        return Detection();
    }
    cv::Rect max_face = faces.at(0);
    for (auto face : faces) {
        if (face.area() > max_face.area()) {
            max_face = face;
        }
    }
    // TODO get detection score
    double score = 0.99;
    Detection detection = Detection(max_face, score);
    detection.box.x *= scale;
    detection.box.y *= scale;
    detection.box.width *= scale;
    detection.box.height *= scale;
    logger->debug("Face: {:03.1f}% at ({} {} {} {})", score*100, max_face.x, max_face.y, max_face.x+max_face.width, max_face.y+max_face.height);

    return detection;
}

void FaceDetector::visualize(cv::Mat* img, const Detection& detection) {
    // draw box
    cv::Scalar color = cv::Scalar(0, 255, 255);

    cv::rectangle(*img, detection.box, color, 1);

    // draw score
    cv::putText(*img, std::to_string((int)(detection.score*100))+" %", cv::Point(detection.box.x, detection.box.y - 20), 0, 0.5, color, 1);

}

cv::Rect FaceDetector::generate_bounding_box(const cv::Rect& face_box, const cv::Mat& img, int w, int h) {
    int x1 = face_box.x + face_box.width;
    int y1 = face_box.y;
    int x2 = std::min(x1 + w, img.size[1]);
    int y2 = std::min(y1 + h, img.size[0]);
    return cv::Rect(cv::Point(x1, y1), cv::Point(x2, y2));
}