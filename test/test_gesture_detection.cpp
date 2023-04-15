#include "gtest/gtest.h"

#include "gesture_detection.h"

TEST(GestureClassifierModelTest, predict) {
    std::string classifier_model = "pretrained_models/resnet18.onnx";
    std::string img_path = "test/data/test_gesture.png";
    cv::dnn::Net classifier_net = cv::dnn::readNet(classifier_model);
    cv::Mat input_img = cv::imread(img_path, cv::IMREAD_COLOR);
    resize_and_pad(input_img, input_img, cv::Size(256, 256));
    cv::Scalar std = { 0.229, 0.224, 0.225 };
    cv::Scalar mean = { 123.675, 116.28, 103.53 }; //[0.485, 0.456, 0.406]*255
    double scale = 1.0 / 255.0;
    std::string class_name = "palm";
    int class_id = 12;
    cv::Mat blob = cv::dnn::blobFromImage(input_img, scale, cv::Size(224, 224), mean, true, true);
    cv::divide(blob, std, blob);
    classifier_net.setInput(blob);
    std::vector<std::string> outNames = classifier_net.getUnconnectedOutLayersNames();
    std::vector<cv::Mat> outs;
    classifier_net.forward(outs, outNames);
    for (auto i : outs) {
        std::cout << "Outs" << i << std::endl << std::endl;
    }
    ASSERT_EQ(0, 0);
}

TEST(GestureDetectorTest, classify) {
    GestureDetector detector = GestureDetector();
    std::string img_path = "test/data/test_gesture.png";
    cv::Mat input_img = cv::imread(img_path, cv::IMREAD_COLOR);
    ClassifierOutput output = detector.classify(input_img);
    std::string class_name = "palm";
    int class_id = 11;
    ASSERT_GE(output.score, 0);
    ASSERT_EQ(output.classId, 0);
}