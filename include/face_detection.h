/**
 * @file face_detection.hpp
 *
 * @brief Face detection implementation.
 *
 * @author Maks Boiar
 *
 */

#ifndef FACE_DETECTION_H
#define FACE_DETECTION_H

#include "detection.h"

using std::string;
using std::vector;
using std::ostream;
using cv::dnn::Net;
using TimePoint = std::chrono::time_point<std::chrono::system_clock>;
using AsyncLogger = std::shared_ptr<spdlog::logger>;


/**
 * Face detection implementation using OpenCV Cascade Classifier.
 */
class FaceDetector {
	cv::CascadeClassifier detector_;
	AsyncLogger logger_;
	double scale_ = 4;
public:
    /**
     * A constructor.
     * @param detector_path path to a XML file with cascade classifier weights.
     */
	FaceDetector(const string& detector_path);

    /**
     * Detect a face with the maximum area in the input image and find its bounding box.
     * @param image matrix containing an image where face is detected.
     * @see visualize()
     * @see generate_bounding_box()
     * @return a `struct` containing a bounding box and confidence
     */
	DetectionResult detect(const image_t& frame);

    /**
     * Put detection score and box on the image.
     * @param img
     * @param detection
     * @param color
     */
	static void visualize(image_t* frame, const DetectionResult& detection, const color_t& color = cv::Scalar(0, 255, 255));
};

#endif