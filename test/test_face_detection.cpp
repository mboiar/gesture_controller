#include "gtest/gtest.h"

#include "face_detection.h"

TEST(FaceDetectorTest, detect) {
    FaceDetector detector = FaceDetector();
    std::string img_path = "test/data/test_face.jpg";
    cv::Mat input_img = cv::imread(img_path, cv::IMREAD_COLOR);
    Detection output = detector.detectAndDisplay(input_img);
    ASSERT_GE(output.score, 0.5);
    ASSERT_GT(output.box.area(), 0);
}