#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <regex>
#include<fstream>
#include <cmath>
#include <argparse.hpp>
#include "spdlog/spdlog.h"
#include "controller.h"
#include <string>
#include <vector>

using std::cerr;
using std::cout;
using std::endl;
using std::vector;
using std::string;


int main(int argc, char* argv[]) {
     argparse::ArgumentParser parser("controller");
     parser.add_argument("-v", "--verbose")
        .help("Display additional information during execution")
        .default_value(false)
        .implicit_value(true);

     parser.add_argument("--log-level")
        .help("Choose logging level")
        .default_value(string("DEBUG"))
        .action([](const string& value) {
            static const vector<string> choices = { "DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL" };
            if (std::find(choices.begin(), choices.end(), value) != choices.end()) {
                return value;
            }
            return string{ "DEBUG" };
        });

     parser.add_argument("--save-video")
        .help("Save video feed to a specified file")
        .default_value(string{ "" });

     parser.add_argument("mode")
        .help("Choose operation mode")
        .action([](const string& value) {
        static const vector<string> choices = { "SIM", "WEBCAM" };
        if (std::find(choices.begin(), choices.end(), value) != choices.end()) {
            return value;
        }
        throw std::invalid_argument("Choose a valid mode option.");
        });

     try {
        parser.parse_args(argc, argv);
     }
     catch (const std::runtime_error& err) {
        cerr << err.what() << endl;
        cerr << parser;
        std::exit(1);
     }


   //   auto verbose = parser.get<bool>("--verbose");
     auto mode = parser.get<string>("mode");
     auto log_level = parser.get<string>("--log-level");
     auto video_filepath = parser.get<string>("--save-video");
     parser.add_description("Control a drone with gestures.");

     spdlog::set_level(spdlog::level::debug);

     Device device;
     device.connect();
     device.streamon();

     std::string gesture_detector_path = "/home/mbcious/copter-gesture/resources/models/resnet18.onnx";
     std::string face_detector_path = "/home/mbcious/copter-gesture/resources/models/haarcascade_frontalface_default.xml";

     Controller controller = Controller(&device, true, face_detector_path, gesture_detector_path);
     controller.run(100);

    return 0;
}
 