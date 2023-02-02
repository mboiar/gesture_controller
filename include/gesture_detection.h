#ifndef GESTURE_DETECTION_H
#define GESTURE_DETECTION_H

#include "detection.h"

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
	BoundingBox box;
	Gesture gesture;
};

class GestureDetector {
public:
	GestureDetector();
	GestureDetection detect(const cv::Mat&);
	void visualize(cv::Mat*, const GestureDetection&);
};

#endif