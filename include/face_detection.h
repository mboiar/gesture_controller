#ifndef FACE_DETECTION_H
#define FACE_DETECTION_H

#include "detection.h"

using std::string;
using std::vector;
using std::ostream;
using cv::dnn::Net;

class FaceDetector {
	Net PNet;
	Net ONet;
	Net RNet;
public:
	FaceDetector();
	Detection detect(const cv::Mat &);
	void visualize(cv::Mat*, const Detection&);
};

#endif