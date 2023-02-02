#ifndef GESTURE_DETECTION_H
#define GESTURE_DETECTION_H

#include "detection.h"

enum class Gesture {
	Left,
	Right,
	Up,
	Down,
	Forward,
	Back,
	Stop,
	Land
};

class GestureDetector {
public:
	Detection detect(const cv::Mat&);
	void visualize(cv::Mat*, const Detection&, const Gesture&);
};

#endif