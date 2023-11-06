#include "gtest/gtest.h"

#include "face_detection.h"

TEST(FaceDetectorTest, detect) {
    FaceDetector detector = FaceDetector("/home/mbcious/copter-gesture/resources/models/haarcascade_frontalface_default.xml");
    std::string img_path = "/home/mbcious/copter-gesture/test/data/test_face.jpg";
    cv::Mat input_img = cv::imread(img_path, cv::IMREAD_COLOR);
    DetectionResult output = detector.detect(input_img);
    ASSERT_GE(output.score, 0.5);
    ASSERT_GT(output.box.area(), 0);
}