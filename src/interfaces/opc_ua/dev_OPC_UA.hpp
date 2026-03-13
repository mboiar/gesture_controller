/**
 * @file if_opc_ua.h
 *
 * @brief OPC UA control interface.
 *
 * @author Maks Boiar
 *
 */

#pragma once

#include <string>
#include <vector>

#include "core/device.hpp"

extern "C" {
#include <dep/open62541.h>
}

/**
 * OPC UA controllable device with a camera.
 */
class OPCUA_Device : public GenericDevice {

public:
  OPCUA_Device() : GenericDevice() { STREAM_URL_ = "udp://0.0.0.0:11111"; }

  void stop() {
    set_opc_value<bool>("::Manual:JogYUp", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
    set_opc_value<bool>("::Manual:JogYDown", false,
                        &UA_TYPES[UA_TYPES_BOOLEAN]);
    set_opc_value<bool>("::Manual:JogXUp", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
    set_opc_value<bool>("::Manual:JogXDown", false,
                        &UA_TYPES[UA_TYPES_BOOLEAN]);
    set_opc_value<bool>("::Manual:JogZUp", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
    set_opc_value<bool>("::Manual:JogZDown", false,
                        &UA_TYPES[UA_TYPES_BOOLEAN]);
  }

  bool get_status() const {
    return get_opc_value<bool>(
        "::AsGlobalPV:gMainInterface.Robot.Status.ServoOK");
  }

  std::string get_mode() const {
    UA_String uastr = get_opc_value<UA_String>("::AsGlobalPV:gModeText");
    char *modeTextVal = (char *)malloc(uastr.length + 1);
    memcpy(modeTextVal, uastr.data, uastr.length);
    modeTextVal[uastr.length] = '\0';
    std::string modeText = modeTextVal;
    free(modeTextVal);
    return std::string(modeText);
  }

  std::vector<double> get_position() {
    std::vector<double> RobotPos(3, 0.0);

    RobotPos[0] = get_opc_value<double>("::AsGlobalPV:MpDelta4Axis_0.X");
    RobotPos[1] = get_opc_value<double>("::AsGlobalPV:MpDelta4Axis_0.Y");
    RobotPos[2] = get_opc_value<double>("::AsGlobalPV:MpDelta4Axis_0.Z");

    return RobotPos;
  }

  template <typename T> T get_opc_value(const std::string &key) const {
    UA_Variant value;
    UA_Variant_init(&value);
    // logger_->info("Reading {}", key);
    auto status = UA_Client_readValueAttribute(
        client, UA_NODEID_STRING(6, const_cast<char *>(key.c_str())), &value);
    // logger_->info("Read {}", key);
    if (status == UA_STATUSCODE_GOOD /* &&
            UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_BOOLEAN])*/) {
      logger_->debug("Read: {}\n", key);
      return *(T *)value.data;
    } else {
      logger_->error("Unable to read value");
    }
    return T{};
  }

  template <typename T>
  void set_opc_value(const std::string &key, T value, const UA_DataType *type) {
    UA_Variant valueVar;
    UA_Variant_setScalar(&valueVar, &value, type);
    auto status = UA_Client_writeValueAttribute(
        client, UA_NODEID_STRING(6, const_cast<char *>(key.c_str())),
        &valueVar);
    if (status == UA_STATUSCODE_GOOD /* &&
            UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_BOOLEAN])*/) {
      logger_->debug("Written: {}\n", key);
    } else {
      logger_->error("Unable to read value");
    }
  }

  void set_opc_bool(const std::string &key, bool value) {
    set_opc_value<bool>(key, value, &UA_TYPES[UA_TYPES_BOOLEAN]);
  }

  void send_rc_control(const velocity_vector_ms_t &velocity);

  int connect(const std::string &server_name) {
    std::string opc_ua_server_name(server_name); // 192.168.137.1
    logger_->info("Connecting to OPC UA server { }", opc_ua_server_name);

    client = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(client));
    UA_StatusCode retval =
        UA_Client_connect(client, opc_ua_server_name.c_str());
    if (retval != UA_STATUSCODE_GOOD) {
      logger_->info("Unable to connect");
      UA_Client_delete(client);
      return (int)retval;
    }
    logger_->info("Connected");

    get_video_stream(0);

    return 0;
  };

  ~OPCUA_Device() {
    UA_Client_disconnect(client);
    UA_Client_delete(client);
  }

private:
  UA_Client *client;
};
