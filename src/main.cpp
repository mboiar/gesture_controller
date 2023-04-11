#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <atomic>
#include <regex>
#include<fstream>
#include <cmath>
#include <ctime>
//#include <sys/stat.h>
//#include <darknet/darknet.h>
//#include <argparse.hpp>
//#include "spdlog/spdlog.h"
//#include "spdlog/sinks/stdout_color_sinks.h"
//#include "controller.h"
#include <chrono>
#include <map>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>

using std::cerr;
using std::cout;
using std::endl;
using std::vector;
using std::string;
using namespace cv;

//class NonZeroLayer : public cv::dnn::Layer
//{
//public:
//	NonZeroLayer(const cv::dnn::LayerParams& params): Layer(params) {
//
//	}
//	static cv::Ptr<cv::dnn::Layer> create(cv::dnn::LayerParams& params) {
//		// create an instance of layer
//		return cv::Ptr<cv::dnn::Layer>(new NonZeroLayer(params));
//	}
//	virtual bool getMemoryShapes(const std::vector<std::vector<int> >& inputs,
//		const int requiredOutputs,
//		std::vector<std::vector<int> >& outputs,
//		std::vector<std::vector<int> >& internals) const CV_OVERRIDE {
//		// output blob's shape computation
//		CV_UNUSED(requiredOutputs); CV_UNUSED(internals);
//		vector<int> outShape(4);
//		outShape[0] = inputs[0][0];  // batch size
//		outShape[1] = inputs[0][1];  // number of channels
//		outShape[2] = outHeight;
//		outShape[3] = outWidth;
//		outputs.assign(1, outShape);
//		return false;
//	}
//	virtual void forward(cv::InputArrayOfArrays inputs,
//		cv::OutputArrayOfArrays outputs,
//		cv::OutputArrayOfArrays internals) CV_OVERRIDE {
//		// run a layer
//		CV_UNUSED(internals);
//		cv::findNonZero(inputs, outputs);
//	}
//	virtual void finalize(cv::InputArrayOfArrays inputs,
//		cv::OutputArrayOfArrays outputs) CV_OVERRIDE;
//};
//
//#include <opencv2/dnn/layer.details.hpp>  // CV_DNN_REGISTER_LAYER_CLASS
//static inline void loadNet()
//{
//	CV_DNN_REGISTER_LAYER_CLASS(NonZero, NonZeroLayer);
//}

void tf_ssd() {
	cv::Mat img = cv::imread("D:\\SolarPlane\\dron_ai\\current\\drone\\test\\testdata\\hand_test_left.jpg");
	auto detector_net1 = cv::dnn::readNet("data/frozen_inference_graph.pb");
	detector_net1.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
	detector_net1.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
	auto blob = cv::dnn::blobFromImage(img, 1.0 / 255.0, cv::Size(256, 256), {}, /* swapRB=*/true, /* crop=*/false);
	detector_net1.setInput(blob);
	vector<string> yolo_layer_names;
	for (const auto& name : detector_net1.getLayerNames())
	{
		if (name.find("detection_") == 0)
		{
			yolo_layer_names.push_back(name);
		}
	}
	vector<vector<cv::Mat>> output_mats;
	detector_net1.forward(output_mats, yolo_layer_names);
}

void torch_ssd() {

}

void yolo() {
	// slow!! but works on cli somehow??
	cv::Mat img = cv::imread("D:\\SolarPlane\\dron_ai\\current\\drone\\test\\testdata\\hand_test_left.jpg");
	auto detector_net1 = cv::dnn::readNet("data/cross-hands.weights", "data/cross-hands.cfg");
	detector_net1.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
	detector_net1.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
	cv::Size detector_dims = cv::Size(0, 0);
	detector_dims.width = 0;
	detector_dims.height = 0;
	int number_of_channels = -1;
	const std::regex rx("^\\s*(channels|width|height)\\s*=\\s*(\\d+)");
	std::ifstream ifs("data/cross-hands.cfg");
	while (ifs.good() && (detector_dims.area() <= 0 || number_of_channels <= 0))
	{
	    std::string line;
	    std::getline(ifs, line);
	    std::smatch sm;
	    if (std::regex_search(line, sm, rx))
	    {
	   	 const std::string key = sm.str(1);
	   	 const int value = std::stoi(sm.str(2));
	   	 if (key == "width")
	   	 {
	   		 detector_dims.width = value;
	   	 }
	   	 else if (key == "height")
	   	 {
	   		 detector_dims.height = value;
	   	 }
	   	 else
	   	 {
	   		 number_of_channels = value;
	   	 }
	    }
	}

	const size_t number_of_classes = 1; //names.size();
	cv::Mat resized; //fast_resize_ignore_aspect_ratio(original_image, network_dimensions);
	cv::resize(img, resized, detector_dims, 0, 0, cv::INTER_NEAREST);
	auto blob = cv::dnn::blobFromImage(resized, 1.0 / 255.0, detector_dims, {}, /* swapRB=*/true, /* crop=*/false);
	detector_net1.setInput(blob);
	vector<string> yolo_layer_names;
	for (const auto& name : detector_net1.getLayerNames())
	{
	    if (name.find("yolo_") == 0)
	    {
	   	 yolo_layer_names.push_back(name);
	    }
	}
	std::vector<std::vector<cv::Mat>> output_mats;
	detector_net1.forward(output_mats, yolo_layer_names);
}

void yolo_darkhelp() {

}

int main(int argc, char* argv[]) {
     //argparse::ArgumentParser parser("Tello_controller");
     //parser.add_argument("-v", "--verbose")
     //    .help("Display additional information during execution")
     //    .default_value(false)
     //    .implicit_value(true);

     //parser.add_argument("--log-level")
     //    .help("Choose logging level")
     //    .default_value(string("INFO"))
     //    .action([](const string& value) {
     //        static const vector<string> choices = { "DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL" };
     //        if (std::find(choices.begin(), choices.end(), value) != choices.end()) {
     //            return value;
     //        }
     //        return string{ "INFO" };
     //    });

     //parser.add_argument("--save-video")
     //    .help("Save video feed to a specified file")
     //    .default_value(string{ "" });

     //parser.add_argument("mode")
     //    .help("Choose operation mode")
     //    .action([](const string& value) {
     //    static const vector<string> choices = { "TELLO-DEBUG", "TELLO", "WEBCAM" };
     //    if (std::find(choices.begin(), choices.end(), value) != choices.end()) {
     //        return value;
     //    }
     //    throw std::invalid_argument("Choose a valid mode option.");
     //    });

     //try {
     //    parser.parse_args(argc, argv);
     //}
     //catch (const std::runtime_error& err) {
     //    cerr << err.what() << endl;
     //    cerr << parser;
     //    std::exit(1);
     //}

     //const char* const TELLO_STREAM_URL{ "udp://0.0.0.0:11111" };

     //auto verbose = parser.get<bool>("--verbose");
     //auto mode = parser.get<string>("mode");
     //auto log_level = parser.get<string>("--log-level");
     //auto video_filepath = parser.get<string>("--save-video");
     //parser.add_description("Control a Tello drone with gestures.");

     //spdlog::set_level(spdlog::level::info);

     //int buffer_len = 5;
     //Tello tello = Tello();
     //tello.connect();
     //tello.streamon();

	 yolo();

     //Controller controller = Controller(&tello, false);
     //controller.run();

    return 0;
}
 