#ifndef DETECTION_H
#define DETECTION_H

#include <iostream>
#include<vector>

#include <opencv2\opencv.hpp>

struct BoundingBox {
	float x1, y1;
	float x2, y2;
	float score;
};

struct Detection {
	//BoundingBox box;
	cv::Rect box = cv::Rect();
	double score = 0;
	Detection(cv::Rect box, double score) : box(box), score(score) {};
	Detection(): box(), score(0) {}
};

#endif // !DETECTION_H
