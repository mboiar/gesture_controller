#include "gtest/gtest.h"

#include "gesture_detection.h"


TEST(GestureDetectorTest, detect) {
    GestureDetector detector = GestureDetector();
    std::string img_path = "test/data/test.png";
    cv::Mat input_img = cv::imread(img_path, cv::IMREAD_COLOR);
    ClassifierOutput output = detector.classify(input_img);
    std::string class_name = "palm";
    ASSERT_GE(output.score, 0.5);
    ASSERT_EQ(output.classId, 10);
}