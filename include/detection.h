#ifndef DETECTION_H
#define DETECTION_H

#include <iostream>
#include<vector>
#include<string>

#include <opencv2\opencv.hpp>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

//struct BoundingBox {
//	float x1, y1;
//	float x2, y2;
//	float score;
//};

struct Detection {
	//BoundingBox box;
	cv::Rect box = cv::Rect();
	double score = 0;
	Detection(cv::Rect box, double score) : box(box), score(score) {};
	Detection(): box(), score(0) {}
};

#endif // !DETECTION_H
