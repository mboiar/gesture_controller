#ifndef DETECTION_H
#define DETECTION_H

#include <iostream>
#include<vector>
#include<string>

#include <opencv2/opencv.hpp>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

using bounding_box_t = cv::Rect;
using score_t = double;
using image_t = cv::Mat;
using color_t = cv::Scalar;

/**
 * Contains bounding box and confidence score for a detection result.
 */
struct DetectionResult {
	bounding_box_t box = cv::Rect();
	score_t score = 0;
	DetectionResult(bounding_box_t box, score_t score) : box(box), score(score) {};
	DetectionResult(): box(), score(0) {}
};

void rescale_box(const bounding_box_t& src, bounding_box_t& dst, double scale);

/**
 * Evaluate softmax function on input array.
 * @param inblob input array
 * @param outblob output array
 */
void softmax(cv::InputArray inblob, cv::OutputArray outblob);

/**
 * Resize an OpenCV image up or down keeping the aspect ratio of the original image constant and padding with specified
 * color if necessary. See https://stackoverflow.com/a/72955620.
 *
 * @param src input image
 * @param dst output image
 * @param new_shape shape of the image after resizing
 * @param pad_color color of the padding region
*/
void resize_and_pad(const image_t& src, image_t& dst, cv::Size new_shape, int pad_color=0);

#endif // !DETECTION_H
