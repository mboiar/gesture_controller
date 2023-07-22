#include "gtest/gtest.h"

#include "gesture_detection.h"

TEST(GestureDetectorTest, detect) {
    std::string img_path = "test/data/test.png";
    cv::Mat input_img = cv::imread(img_path, cv::IMREAD_COLOR);
    GestureDetector detector = GestureDetector("resources/models/resnet18.onnx");
    ClassifierOutput output = detector.classify(input_img);
    std::string class_name = "palm";
    ASSERT_GE(output.score, 0.5);
    ASSERT_EQ(output.classId, 10);
}