#include "dev_OPC_UA.hpp"

using std::string;
using std::vector;

void OPCUA_Device::send_rc_control(const velocity_vector_ms_t &vel) {
  logger_->info("rc {} {} {} {}", vel.at(0), vel.at(1), vel.at(2), vel.at(3));

  if (vel.at(0) < 0) {
    set_opc_bool("::Manual:JogYUp", true);
    set_opc_bool("::Manual:JogYDown", false);

  } else if (vel.at(0) > 0) {
    set_opc_bool("::Manual:JogYUp", false);
    set_opc_bool("::Manual:JogYDown", true);
  } else {
    set_opc_bool("::Manual:JogYUp", false);
    set_opc_bool("::Manual:JogYDown", false);
  }

  if (vel.at(1) < 0) {
    set_opc_bool("::Manual:JogXUp", true);
    set_opc_bool("::Manual:JogXDown", false);

  } else if (vel.at(1) > 0) {
    set_opc_bool("::Manual:JogXUp", false);
    set_opc_bool("::Manual:JogXDown", true);
  } else {
    set_opc_bool("::Manual:JogYUp", false);
    set_opc_bool("::Manual:JogYDown", false);
  }
  if (vel.at(2) < 0) {
    set_opc_bool("::Manual:JogZUp", true);
    set_opc_bool("::Manual:JogZDown", false);

  } else if (vel.at(2) > 0) {
    set_opc_bool("::Manual:JogZUp", false);
    set_opc_bool("::Manual:JogZDown", true);
  } else {
    set_opc_bool("::Manual:JogZUp", false);
    set_opc_bool("::Manual:JogZDown", false);
  }
}

void OPCUA_Device::stop() {
  set_opc_value<bool>("::Manual:JogYUp", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
  set_opc_value<bool>("::Manual:JogYDown", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
  set_opc_value<bool>("::Manual:JogXUp", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
  set_opc_value<bool>("::Manual:JogXDown", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
  set_opc_value<bool>("::Manual:JogZUp", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
  set_opc_value<bool>("::Manual:JogZDown", false, &UA_TYPES[UA_TYPES_BOOLEAN]);
}

std::string OPCUA_Device::get_mode() const {
  UA_String uastr = get_opc_value<UA_String>("::AsGlobalPV:gModeText");
  char *modeTextVal = (char *)malloc(uastr.length + 1);
  memcpy(modeTextVal, uastr.data, uastr.length);
  modeTextVal[uastr.length] = '\0';
  std::string modeText = modeTextVal;
  free(modeTextVal);
  return std::string(modeText);
}

std::vector<double> OPCUA_Device::get_position() const {
  std::vector<double> RobotPos(3, 0.0);

  RobotPos[0] = get_opc_value<double>("::AsGlobalPV:MpDelta4Axis_0.X");
  RobotPos[1] = get_opc_value<double>("::AsGlobalPV:MpDelta4Axis_0.Y");
  RobotPos[2] = get_opc_value<double>("::AsGlobalPV:MpDelta4Axis_0.Z");

  return RobotPos;
}

template <typename T>
T OPCUA_Device::get_opc_value(const std::string &key) const {
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
void OPCUA_Device::set_opc_value(const std::string &key, T value,
                                 const UA_DataType *type) {
  UA_Variant valueVar;
  UA_Variant_setScalar(&valueVar, &value, type);
  auto status = UA_Client_writeValueAttribute(
      client, UA_NODEID_STRING(6, const_cast<char *>(key.c_str())), &valueVar);
  if (status == UA_STATUSCODE_GOOD /* &&
            UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_BOOLEAN])*/) {
    logger_->debug("Written: {}\n", key);
  } else {
    logger_->error("Unable to read value");
  }
}

int OPCUA_Device::connect(const std::string &server_name) {
  std::string opc_ua_server_name(server_name); // 192.168.137.1
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