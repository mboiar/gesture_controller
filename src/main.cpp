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
#include <argparse.hpp>
#include "spdlog/spdlog.h"
//#include "spdlog/sinks/stdout_color_sinks.h"
#include "controller.h"
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

     //const char* const TELLO_STREAM_URL{ "udp://0.0.0.0:11111" };

     auto verbose = parser.get<bool>("--verbose");
     auto mode = parser.get<string>("mode");
     auto log_level = parser.get<string>("--log-level");
     auto video_filepath = parser.get<string>("--save-video");
     parser.add_description("Control a drone with gestures.");

     spdlog::set_level(spdlog::level::debug);

     int buffer_len = 5;
     Drone drone = Drone();
     drone.connect();
     drone.streamon();

     Controller controller = Controller(&drone, false);
     controller.run(100);

    return 0;
}
 