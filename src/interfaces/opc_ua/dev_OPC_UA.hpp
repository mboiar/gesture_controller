/**
 * @file dev_OPC_UA.hpp
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

  void stop();

  bool get_status() const {
    return get_opc_value<bool>(
        "::AsGlobalPV:gMainInterface.Robot.Status.ServoOK");
  }

  std::string get_mode() const;

  std::vector<double> get_position() const;

  template <typename T> T get_opc_value(const std::string &key) const;

  template <typename T>
  void set_opc_value(const std::string &key, T value, const UA_DataType *type);

  void set_opc_bool(const std::string &key, bool value) {
    set_opc_value<bool>(key, value, &UA_TYPES[UA_TYPES_BOOLEAN]);
  }

  void send_rc_control(const velocity_vector_ms_t &velocity);

  int connect(const std::string &server_name);

  ~OPCUA_Device() {
    UA_Client_disconnect(client);
    UA_Client_delete(client);
  }

private:
  UA_Client *client;
};
