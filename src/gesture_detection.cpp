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
    const string &classifier_model
) {
    string name("DETECTION");
    logger = spdlog::get(name);
    if (!logger) {
        logger = spdlog::stdout_color_mt(name);
    }
    logger->set_level(spdlog::level::debug);

    classifier_net = cv::dnn::readNet(classifier_model);
}

void softmax(cv::InputArray inblob, cv::OutputArray outblob)
    {
        const cv::Mat input = inblob.getMat();
        outblob.create(inblob.size(), inblob.type());

        cv::Mat exp;
        const float max = *std::max_element(input.begin<float>(), input.end<float>());
        cv::exp((input - max), exp);
        outblob.getMat() = exp / cv::sum(exp)[0];
	}

void resize_and_pad(cv::Mat& src, cv::Mat& dst, cv::Size new_shape, int pad_color) {
    /**
     * Resize an OpenCV image up or down keeping the aspect ratio of the original image constant and padding with specified
     * color if necessary. See https://stackoverflow.com/a/72955620.
    */
    int src_width = src.cols;
    int src_height = src.rows;
    int new_width = new_shape.width;
    int new_height = new_shape.height;
    int interpolation_method;
    int pad_top, pad_bottom, pad_left, pad_right;
    // float aspect_ratio = float(new_width / std::max(width, height));
    float aspect_ratio = float(src_width) / src_height;
    float new_aspect_ratio = float(new_width) / new_height;
    if (src_height > new_height || src_width > new_width){
        interpolation_method = cv::INTER_AREA;
    } else {
        interpolation_method = cv::INTER_CUBIC;
    }
    if ( (new_aspect_ratio >= aspect_ratio) || ((new_aspect_ratio == 1) && (aspect_ratio <= 1)) ){
        // new_height = new_height;
        new_width = int(new_height * aspect_ratio);
        pad_left = int(float(new_shape.width - new_width) / 2);
        pad_right = int(float(new_shape.width - new_width) / 2);
        pad_top = 0;
        pad_bottom = 0;
    }    
    else {
        // new_width = new_width;
        new_height = int(new_width / aspect_ratio);
        pad_top = int(float(new_shape.height - new_height) / 2);
        pad_bottom = int(float(new_shape.height - new_width) / 2);
        pad_left = 0;
        pad_right = 0;
    }

    cv::resize(src, dst, cv::Size(new_width, new_height), 0, 0, interpolation_method);

    cv::Scalar color = cv::Scalar(pad_color, pad_color, pad_color);
    cv::copyMakeBorder(dst, dst, pad_top, pad_bottom, pad_left, pad_right, cv::BORDER_CONSTANT | CV_HAL_BORDER_ISOLATED, color);
}

ClassifierOutput GestureDetector::classify(const cv::Mat &img) {
   // rescale frame to [0, 1] than resize and pad to [224, 224, 3]
    double scale = 1.0 / 255.0;
    int inpWidth = 224;
    int inpHeight = 224;
    // logger->info("Bur-bur");
    cv::Mat resized_img = img;
    resize_and_pad(resized_img, resized_img, cv::Size(inpWidth, inpHeight));
    // cv::imwrite("test.png", resized_img);
    // logger->info("Bur-bur-bur");
    cv::Mat blob = cv::dnn::blobFromImage(resized_img, scale, cv::Size(inpWidth, inpHeight), 0, true, false);
    // logger->info("Bur-bur-bur-bur");

    classifier_net.setInput(blob);
    std::vector<std::string> outNames = classifier_net.getUnconnectedOutLayersNames();
    std::vector<cv::Mat> outs;
    classifier_net.forward(outs, outNames);
    for (auto i : outs) {
        softmax(i, i);
        std::cout << "Outs" << i << std::endl << std::endl;
    }
    cv::Point classIdPoint_leading_hand;
    double confidence_leading_hand;
    cv::Point classIdPoint_gesture;
    double confidence_gesture;
    cv::minMaxLoc(outs.at(1).reshape(1, 1), 0, &confidence_gesture, 0, &classIdPoint_gesture);
    logger->info("Gesture class: {} conf: {:.2f}", classIdPoint_gesture.x, confidence_gesture);
    // Gesture classId = static_cast<Gesture>(classIdPoint_gesture.x);
    // ClassifierOutput classified_gesture = ClassifierOutput(confidence, classId);
    cv::minMaxLoc(outs.at(0).reshape(1, 1), 0, &confidence_leading_hand, 0, &classIdPoint_leading_hand);
    logger->info("Leading hand: {} conf: {:.2f}", classIdPoint_leading_hand.x, confidence_leading_hand);

    // logger->info("Gesture {}: {:.2f}%", classified_gesture.classId, classified_gesture.score);
    ClassifierOutput classified_gesture = ClassifierOutput(confidence_gesture, classIdPoint_gesture.x);
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
