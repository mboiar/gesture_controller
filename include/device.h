/**
 * @file device.h
 *
 * @brief Generic controllable device interface.
 *
 * @author Maks Boiar
 *
 */

#ifndef DEVICE_H
#define DEVICE_H

#include <vector>
#include <string>
#include <chrono>
#include <atomic>

#include <opencv2/opencv.hpp>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

extern "C" {
#include <open62541/client_config_default.h>
#include <open62541/client_highlevel.h>

}

using std::string;
using std::vector;
using AsyncLogger = std::shared_ptr<spdlog::logger>;

using velocity_vector_ms_t = vector<int>;

/**
 * List of command names for device control.
 */
enum Command {
    Idle,
    JogXUp,  // dislike
    JogXDown, // fist
    JogYUp,  // four
    JogYDown,  // like
    ToolOn,  // ?
    JogZDown,  // ok
    Stop,  // mute
    JogZUp,  // palm
    ToolOff,
    NoGesture = 18
};

/**
 * Abstract controllable device with a camera.
 */
class Device {
	AsyncLogger logger_;
	bool simulate_;
    static const char STREAM_URL_[];
    UA_Client* client;
    cv::VideoCapture cap;
public:
    Device(bool simulate = true): simulate_(simulate) {
        string name("DEVICE");
        logger_ = spdlog::get(name);
        if (!logger_) {
            logger_ = spdlog::stdout_color_mt(name);
        }
        //logger_->set_level(spdlog::level::info);
    }
	int get_battery();
    template<typename T>
    T get_opc_value(char * key) {
        UA_Variant value;
        UA_Variant_init(&value);
        //logger_->info("Reading {}", key);
        auto status = UA_Client_readValueAttribute(client, UA_NODEID_STRING(6, key), &value);
        //logger_->info("Read {}", key);
        if (status == UA_STATUSCODE_GOOD /* &&
            UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_BOOLEAN])*/) {
            logger_->debug("Read: {}\n", key);
            return *(T*)value.data;
        }
        else {
            logger_->error("Unable to read value");
        }
        return T{};
    }
    template<typename T>
    void set_opc_value(char* key, T value, const UA_DataType* type) {
        UA_Variant valueVar;
        UA_Variant_setScalar(&valueVar, &value, type);
        auto status = UA_Client_writeValueAttribute(client, UA_NODEID_STRING(6, key), &valueVar);
        if (status == UA_STATUSCODE_GOOD /* &&
            UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_BOOLEAN])*/) {
            logger_->debug("Written: {}\n", key);
        }
        else {
            logger_->error("Unable to read value");
        }
    }
    //template<typename T>
    //void set_opc_array(char* key, T& value, size_t size, const UA_DataType* type) {
    //    UA_Variant valueVar;
    //    UA_Variant_init(&variant);
    //    UA_Variant_setArray(&variant, value, size, type);

    //    auto status = UA_Client_writeValueAttribute(client, UA_NODEID_STRING(6, key), &valueVar);
    //    if (status == UA_STATUSCODE_GOOD /* &&
    //        UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_BOOLEAN])*/) {
    //        logger_->debug("Written: {}={}\n", key, *(bool*)value.data);
    //    }
    //    else {
    //        logger_->error("Unable to read value");
    //    }
    //}

	void send_rc_control(const velocity_vector_ms_t& velocity);
	void land();

 //   /**
 //    * Capture video stream.
 //    *
 //    * @param camera_id id of the camera whose stream will be captured
 //    */
	cv::VideoCapture get_video_stream(int camera_id);

 //   /**
 //    * Enable video streaming.
 //    */
	void streamon(){};

 //   /**
 //    * Connect to a device.
 //    */
	int connect(const std::string& server_name){
        std::string opc_ua_server_name(server_name);  // 192.168.137.1
        logger_->info("Connecting to OPC UA server { }", opc_ua_server_name);

        client = UA_Client_new();
        UA_ClientConfig_setDefault(UA_Client_getConfig(client));
        UA_StatusCode retval = UA_Client_connect(client, opc_ua_server_name.c_str());
        if (retval != UA_STATUSCODE_GOOD) {
            logger_->info("Unable to connect");
            UA_Client_delete(client);
            return (int)retval;
        }
        logger_->info("Connected");

        get_video_stream(0);

        return 0;
    };

    void get_frame(cv::Mat* frame) { cap >> *frame; }

    ~Device() {
        UA_Client_disconnect(client);
        UA_Client_delete(client);
    }
};

#endif