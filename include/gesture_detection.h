/**
 * @file gesture_detection.hpp
 *
 * @brief Gesture detection implementation.
 *
 * @author Maks Boiar
 *
 */

#ifndef GESTURE_DETECTION_H
#define GESTURE_DETECTION_H

#include "detection.h"
#include <map>

using AsyncLogger = std::shared_ptr<spdlog::logger>;
using std::vector;
using std::string;
using landmarks_t = std::vector<double>;


struct ClassifierOutput {
	double score = 0;
	int class_id = 0;
	landmarks_t landmarks;
	ClassifierOutput(double score, int class_id) : score(score), class_id(class_id) {}
	ClassifierOutput() : score(0), class_id(0) {}
};

/**
 * Gesture detection implementation based on a ResNet model.
 */
class GestureDetector {
	AsyncLogger logger_;
	cv::dnn::Net detector_;
public:
    /**
     * A constructor.
     * @param detector_path path to a ONNX model.
     */
	GestureDetector(const string& detector_path);
	static void visualize(
            image_t* image, const ClassifierOutput& classified_gesture,
            const bounding_box_t& gesture_box, const color_t& color = cv::Scalar(0, 255, 255)
                    );
	ClassifierOutput detect(const cv::Mat&);
    static bounding_box_t get_detection_area(const bounding_box_t& face_box, int img_width, int img_height, int w, int h);
    static cv::Mat preprocess_image(const image_t& img);
};

#endif