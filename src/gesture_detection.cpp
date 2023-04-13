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

GestureDetector::GestureDetector(
    //const string &detector_model, 
    const string &classifier_model
) {
    string name("DETECTION");
    logger = spdlog::get(name);
    if (!logger) {
        logger = spdlog::stdout_color_mt(name);
    }
    logger->set_level(spdlog::level::info);

    classifier_net = cv::dnn::readNet(classifier_model);
    //ssdlite = cv::dnn::DetectionModel::DetectionModel("data/ssdlite.onnx");
    // detector_net = cv::dnn::DetectionModel::DetectionModel("data/cross-hands.weights", "data/cross-hands.cfg");
    // detector_net1 = cv::dnn::readNet("data/cross-hands.weights", "data/cross-hands.cfg");
    // detector_net1.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
    // detector_net1.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
    // detector_dims = cv::Size(0, 0);
    // detector_dims.width = 256;
    // detector_dims.height = 256;

}

struct Resize
{
    cv::Mat resized_image;
    int dw;
    int dh;
};

Resize resize_and_pad(cv::Mat& img, cv::Size new_shape) {
    float width = img.cols;
    float height = img.rows;
    float r = float(new_shape.width / std::max(width, height));
    int new_unpadW = int(round(width * r));
    int new_unpadH = int(round(height * r));
    Resize resize;
    cv::resize(img, resize.resized_image, cv::Size(new_unpadW, new_unpadH), 0, 0, cv::INTER_AREA);

    resize.dw = new_shape.width - new_unpadW;
    resize.dh = new_shape.height - new_unpadH;
    cv::Scalar color = cv::Scalar(100, 100, 100);
    cv::copyMakeBorder(resize.resized_image, resize.resized_image, 0, resize.dh, 0, resize.dw, cv::BORDER_CONSTANT, color);

    return resize;
}

void chw_to_hwc(cv::InputArray src, cv::OutputArray dst) {
    const auto& src_size = src.getMat().size;
    const int src_c = src_size[0];
    const int src_h = src_size[1];
    const int src_w = src_size[2];

    auto c_hw = src.getMat().reshape(0, { src_c, src_h * src_w });

    dst.create(src_h, src_w, CV_MAKETYPE(src.depth(), src_c));
    cv::Mat dst_1d = dst.getMat().reshape(src_c, { src_h, src_w });

    cv::transpose(c_hw, dst_1d);
}

cv::Mat fast_resize(cv::Mat mat, const cv::Size& desired_size) {

    const double image_width = static_cast<double>(mat.cols);
    const double image_height = static_cast<double>(mat.rows);
    const double horizontal_factor = image_width / static_cast<double>(desired_size.width);
    const double vertical_factor = image_height / static_cast<double>(desired_size.height);
    const double largest_factor = std::max(horizontal_factor, vertical_factor);
    const double new_width = image_width / largest_factor;
    const double new_height = image_height / largest_factor;
    const cv::Size new_size(std::round(new_width), std::round(new_height));

    // "To shrink an image, it will generally look best with CV_INTER_AREA interpolation ..."
    auto interpolation = cv::INTER_AREA;
    if (largest_factor < 1.0)
    {
        // "... to enlarge an image, it will generally look best with CV_INTER_CUBIC"
        interpolation = cv::INTER_CUBIC;
    }

    cv::Mat dst;
    cv::resize(mat, dst, new_size, 0, 0, interpolation);

    return dst;
}

ClassifierOutput GestureDetector::classify(const cv::Mat &img) {
   // preprocess frame
    int rszWidth = 256;
    int rszHeight = 256;
    cv::Scalar std = { 0.229, 0.224, 0.225 };
    cv::Scalar mean = { 123.675, 116.28, 103.53 };//[0.485, 0.456, 0.406]*255
    double scale = 1;
    int inpWidth = 224;
    int inpHeight = 224;
    // logger->info("Bur-bur");
    // cv::resize(img, img, cv::Size(rszWidth, rszHeight));
    // logger->info("Bur-bur-bur");
    cv::Mat blob = cv::dnn::blobFromImage(img, scale, cv::Size(inpWidth, inpHeight), mean, true, true);
    // logger->info("Bur-bur-bur-bur");

    cv::divide(blob, std, blob);

    classifier_net.setInput(blob);
    std::vector<std::string> outNames = classifier_net.getUnconnectedOutLayersNames();
    std::vector<cv::Mat> outs;
    classifier_net.forward(outs, outNames);
    for (auto i : outs) {
        std::cout << "Outs" << i << std::endl << std::endl;
    }
    // logger->info("{}", outs.size());
    logger->debug("Output layers: {}", fmt::join(outNames, ", "));
    //cv::Mat prob = classifier_net.forward();
    //std::cout << prob << std::endl;
    cv::Point classIdPoint;
    double confidence;
    cv::minMaxLoc(outs.at(0).reshape(1, 1), 0, &confidence, 0, &classIdPoint);
    logger->info("{} ({} {})", confidence, classIdPoint.x, classIdPoint.y);
    Gesture classId = static_cast<Gesture>(classIdPoint.x);
    ClassifierOutput classified_gesture = ClassifierOutput(confidence, classId);
    cv::minMaxLoc(outs.at(1).reshape(1, 1), 0, &confidence, 0, &classIdPoint);
    logger->info("{} ({} {})", confidence, classIdPoint.x, classIdPoint.y);

    logger->info("Gesture {}: {:03.1f}%", classified_gesture.classId, classified_gesture.score*100);
    // ClassifierOutput classified_gesture = ClassifierOutput();
    return classified_gesture;
}

void GestureDetector::visualize(cv::Mat* img, const ClassifierOutput& classified_gesture, const cv::Rect& gesture_box) {
    cv::Scalar color = cv::Scalar(0, 255, 255);
    // draw gesture name
    cv::putText(*img, gesture_map[classified_gesture.classId], cv::Point(gesture_box.x + 80, gesture_box.y - 20), 0, 0.5, color, 1);
    // draw landmarks
    if (!classified_gesture.landmarks.empty()) {
        // TODO draw gesture landmarks
    }
    // draw score
    cv::putText(*img, std::to_string((int)(classified_gesture.score * 100)) + " %", cv::Point(gesture_box.x, gesture_box.y - 20), 0, 0.5, color, 1);

}
