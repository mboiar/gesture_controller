#include "spdlog/spdlog.h"
#include <algorithm>
#include <cmath>
#include <dep/argparse.hpp>
#include <fstream>
#include <iostream>
#include <regex>
#include <stdexcept>
#include <string>
#include <vector>

#include "core/controller.hpp"
#include <interfaces/opc_ua/dev_OPC_UA.hpp>

using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::vector;

int main(int argc, char *argv[]) {

  std::cout << "test" << std::endl;

  auto logger_ = spdlog::get("MAIN");

  if (!logger_) {
    logger_ = spdlog::stdout_color_mt("MAIN");
  }
  logger_->set_level(spdlog::level::info);

  logger_->info("Parsing input arguments");

  argparse::ArgumentParser parser("controller");
  parser.add_argument("-v", "--verbose")
      .help("Display additional information during execution")
      .default_value(false)
      .implicit_value(true);

  parser.add_argument("--log-level")
      .help("Choose logging level")
      .scan<'d', int>()
      .default_value(0);

  parser.add_argument("--save-video")
      .help("Save video feed to a specified file")
      .default_value(string{""});

  parser.add_argument("mode")
      .help("Choose operation mode")
      .action([](const string &value) {
        static const vector<string> choices = {"SIM", "WEBCAM"};
        if (std::find(choices.begin(), choices.end(), value) != choices.end()) {
          return value;
        }
        throw std::invalid_argument("Choose a valid mode option.");
      });

  parser.add_argument("--server-path")
      .help("Save video feed to a specified file")
      .default_value(string{"127.0.0.1:4840"});

  parser.add_description("Control a delta robot with gestures.");

  try {
    parser.parse_args(argc, argv);
  } catch (const std::runtime_error &err) {
    logger_->error(err.what());
    return EXIT_FAILURE;
  }

  string mode = parser.get<string>("mode");
  string server_addr = parser.get<string>("--server-path");
  int log_level = parser.get<int>("--log-level");
  string video_filepath = parser.get<string>("--save-video");

  spdlog::set_level(static_cast<spdlog::level::level_enum>(log_level));

  logger_->info("Connecting to device");
  std::string opc_ua_server_name = "opc.tcp://" + server_addr;
  Device device = OPCUA_Device{};
  if (device.connect(opc_ua_server_name) < 0) {
    logger_->info("Program exited with status {}", EXIT_FAILURE);
    return EXIT_FAILURE;
  }
  device.streamon();

  std::string gesture_detector_path = "../resources/models/resnet18.onnx";
  std::string face_detector_path =
      "../resources/models/haarcascade_frontalface_default.xml";

  Controller controller =
      Controller(&device, true, face_detector_path, gesture_detector_path);
  logger_->info("Running");
  controller.run(50);

  logger_->info("Program exited with status {}", EXIT_SUCCESS);
  return EXIT_SUCCESS;
}
