#include <algorithm>

#include <D:\cpplibs\argparse\include\argparse\argparse.hpp>

#include "controller.h"

using std::cerr;
using std::cout;
using std::endl;
using std::vector;
using std::string;

int main(int argc, char* argv[]) {
     argparse::ArgumentParser parser("Tello_controller");
     parser.add_argument("-v", "--verbose")
         .help("Display additional information during execution")
         .default_value(false)
         .implicit_value(true);

     parser.add_argument("--log-level")
         .help("Choose logging level")
         .default_value(string("INFO"))
         .action([](const string& value) {
             static const vector<string> choices = { "DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL" };
             if (std::find(choices.begin(), choices.end(), value) != choices.end()) {
                 return value;
             }
             return string{ "INFO" };
         });

     parser.add_argument("--save-video")
         .help("Save video feed to a specified file")
         .default_value(string{ "" });

     parser.add_argument("mode")
         .help("Choose operation mode")
         .action([](const string& value) {
         static const vector<string> choices = { "TELLO-DEBUG", "TELLO", "WEBCAM" };
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

     auto verbose = parser.get<bool>("--verbose");
     auto mode = parser.get<string>("mode");
     auto log_level = parser.get<string>("--log-level");
     auto video_filepath = parser.get<string>("--save-video");
     parser.add_description("Control a Tello drone with gestures.");

     int buffer_len = 5;
     Tello tello = Tello();
     // TODO setup tello
     Controller controller = Controller(tello, false);
     controller.run();

    return 0;
}

// TODO check that const T& is used (unmodified variable)
//                 T* (modified variable)

    // if mode == "webcam":
    //     tello = DummyTello()
    // else:
    //     tello = Tello()

    // tello.connect()
    // tello.streamon()
    // debug = True if mode == "tello-debug" else False
    // buffer_len = 5
    // DetectionRunner = TelloDetectionRunner(
    //     tello,
    //     buffer_len,
    //     debug,
    //     detection_logging_level=detection_log_level,
    //     controller_logging_level=controller_log_level,
    //     save_video=save_video,
    // )
    // DetectionRunner.run()
