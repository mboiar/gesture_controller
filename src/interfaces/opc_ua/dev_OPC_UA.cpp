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
