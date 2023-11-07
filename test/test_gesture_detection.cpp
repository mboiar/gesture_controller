#include "gtest/gtest.h"

#include "gesture_detection.h"

TEST(GestureDetectorTest, DISABLED_detect) {
    std::string img_path = "/home/mbcious/copter-gesture/test/data/test.png";
    cv::Mat input_img = cv::imread(img_path, cv::IMREAD_COLOR);
    GestureDetector detector = GestureDetector("/home/mbcious/copter-gesture/resources/models/resnet18.onnx");
    ClassifierOutput output = detector.detect(input_img);
    std::string class_name = "palm";
    ASSERT_GE(output.score, 0.5);
    ASSERT_EQ(output.class_id, 10);
}