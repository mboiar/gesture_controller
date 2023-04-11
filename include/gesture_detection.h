#ifndef GESTURE_DETECTION_H
#define GESTURE_DETECTION_H

#include "detection.h"
#include <map>
//#include "GRT/GRT.h"
//#include "darkhelp/DarkHelp.hpp"

using AsyncLogger = std::shared_ptr<spdlog::logger>;
using std::vector;
using std::string;

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


struct ClassifierOutput {
	double score = 0;
	int classId = 0;
	vector<double> landmarks;
	ClassifierOutput(double score, int classId) : score(score), classId(classId) {}
	ClassifierOutput(double score, int classId, const vector<double>& landmarks) : score(score), classId(classId), landmarks(landmarks) {}
	ClassifierOutput() : score(0), classId(0) {}
};


class GestureDetector {
	double scale = 1.0;
	AsyncLogger logger;
	cv::dnn::Net classifier_net;
	cv::dnn::DetectionModel detector_net;
	cv::dnn::Net detector_net1;
	cv::Size detector_dims;
	cv::dnn::DetectionModel ssdlite;
	//DarkHelp::NN nn;
public:
	GestureDetector(
		//const string& detector_model = "data/ssdlite.onnx",
		const string& classifier_model = "data/resnet18.onnx"
	);
	Detection detect(const cv::Mat&);
	void visualize(cv::Mat*, const ClassifierOutput&, const cv::Rect&);
	ClassifierOutput classify(const cv::Mat&);
};

#endif