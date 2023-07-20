#include "gtest/gtest.h"

#include "gesture_detection.h"

TEST(GestureClassifierModelTest, predict) {
    std::string classifier_model = "pretrained_models/resnet18.onnx";
    std::string img_path = "test/data/test.png";
    cv::dnn::Net classifier_net = cv::dnn::readNet(classifier_model);
    cv::Mat input_img = cv::imread(img_path, cv::IMREAD_COLOR);
    resize_and_pad(input_img, input_img, cv::Size(224, 224));
    // cv::Scalar std = { 0.229, 0.224, 0.225 };
    // cv::Scalar mean = { 123.675, 116.28, 103.53 }; //[0.485, 0.456, 0.406]*255
    double scale = 1.0 / 255.0;
    std::string class_name = "rock";
    int class_id = 8;
    cv::Mat blob = cv::dnn::blobFromImage(input_img, scale, cv::Size(224, 224), 0, true, false);
    // cv::divide(blob, std, blob);
    classifier_net.setInput(blob);
    std::vector<std::string> outNames = classifier_net.getUnconnectedOutLayersNames();
    std::vector<cv::Mat> outs;
    classifier_net.forward(outs, outNames);
    for (auto i : outs) {
        softmax(i, i);
        std::cout << "Outs" << i << std::endl << std::endl;
    }
    cv::Point classIdPoint_leading_hand;
    double confidence_leading_hand;
    cv::Point classIdPoint_gesture;
    double confidence_gesture;
    cv::minMaxLoc(outs.at(0).reshape(1, 1), 0, &confidence_gesture, 0, &classIdPoint_gesture);
    std::cout << "Leading hand:" << classIdPoint_gesture.x << " " << confidence_gesture << std::endl;
    // Gesture classId = static_cast<Gesture>(classIdPoint_gesture.x);
    // ClassifierOutput classified_gesture = ClassifierOutput(confidence, classId);
    cv::minMaxLoc(outs.at(1).reshape(1, 1), 0, &confidence_leading_hand, 0, &classIdPoint_leading_hand);
    std::cout << "Gesture class: " << classIdPoint_leading_hand.x << " " << confidence_leading_hand << std::endl;

    ASSERT_GT(confidence_gesture, 0.5);
    ASSERT_GT(confidence_leading_hand, 0.5);

    ASSERT_EQ(classIdPoint_gesture.x, 1);
    ASSERT_EQ(classIdPoint_leading_hand.x, 10);


}

TEST(GestureDetectorTest, classify) {
    GestureDetector detector = GestureDetector();
    std::string img_path = "test/data/test.png";
    cv::Mat input_img = cv::imread(img_path, cv::IMREAD_COLOR);
    ClassifierOutput output = detector.classify(input_img);
    std::string class_name = "palm";
    int class_id = 11;
    ASSERT_GE(output.score, 0);
    ASSERT_EQ(output.classId, 0);
}