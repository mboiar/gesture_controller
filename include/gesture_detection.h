#ifndef GESTURE_DETECTION_H
#define GESTURE_DETECTION_H

#include "detection.h"
#include <map>

using AsyncLogger = std::shared_ptr<spdlog::logger>;

enum Gesture {
	NoGesture = 0,
	Left,
	Right,
	Up,
	Down,
	Forward,
	Back,
	Stop,
	Land,
	GestureCount
};


struct GestureDetection {
	//cv::Rect box = cv::Rect();
	double score = 0;
	Gesture gesture = NoGesture;
	GestureDetection(double score, Gesture gesture) : score(score), gesture(gesture) {}
	GestureDetection() : score(0), gesture(NoGesture) {}
};

class GestureDetector {
	int scale = 4;
	AsyncLogger logger;
public:
	GestureDetector();
	GestureDetection detect(const cv::Mat&);
	void visualize(cv::Mat*, const GestureDetection&, const cv::Rect&);
};

#endif