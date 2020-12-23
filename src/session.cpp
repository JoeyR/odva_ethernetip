/**
Software License Agreement (BSD)

\file      session.cpp
\authors   Kareem Shehata <kareem@shehata.ca>
\copyright Copyright (c) 2015, Clearpath Robotics, Inc., All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.
 * Neither the name of Clearpath Robotics nor the names of its contributors may
be used to endorse or promote products derived from this software without
specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WAR- RANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, IN- DIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "odva_ethernetip/session.h"

#include <random>

#include "odva_ethernetip/eip_types.h"
#include "odva_ethernetip/encap_packet.h"
#include "odva_ethernetip/register_session_data.h"
#include "odva_ethernetip/rr_data_request.h"
#include "odva_ethernetip/rr_data_response.h"
#include "odva_ethernetip/serialization/buffer_reader.h"
#include "odva_ethernetip/serialization/buffer_writer.h"
#include <boost/bind.hpp>
#include <boost/make_shared.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/shared_ptr.hpp>

using boost::make_shared;
using boost::shared_ptr;
using std::endl;

namespace eip {

using serialization::BufferReader;
using serialization::BufferWriter;

Session::Session(shared_ptr<Socket> socket, shared_ptr<Socket> io_socket,
                 EIP_UINT vendor_id, EIP_UDINT serial_num)
    : socket_(socket),
      io_socket_(io_socket),
      session_id_(0),
      my_vendor_id_(vendor_id),
      my_serial_num_(serial_num) {
  // generate pseudo-random connection ID and connection SN starting points
  static std::random_device dev;
  static std::mt19937 rng(dev());
  boost::random::uniform_int_distribution<> dist(0, 0xFFFF);
  next_connection_id_ = dist(rng);
  next_connection_sn_ = dist(rng);
  // printf("Generated starting connection ID %zu and SN %zu\n",
  //       next_connection_id_, next_connection_sn_);
}

Session::~Session() {
  try {
    if (session_id_ != 0) {
      close();
    }
  } catch (...) {
    // can't throw exceptions, but can't do anything either
  }
}

void Session::open(string hostname, string port, string io_port) {
  // printf("Resolving hostname and connecting socket\n");
  socket_->open(hostname, port);
  io_socket_->open(hostname, io_port);

  // create the registration message
  // printf("Creating and sending the registration message\n");
  shared_ptr<RegisterSessionData> reg_data = make_shared<RegisterSessionData>();
  EncapPacket reg_msg(EIP_CMD_REGISTER_SESSION, 0, reg_data);

  // send the register session message and get response
  EncapPacket response;
  try {
    response = sendCommand(reg_msg);
  } catch (std::length_error ex) {
    socket_->close();
    io_socket_->close();
    // printf("Could not parse response when registering session: %s\n",
    //       ex.what());
    throw std::runtime_error("Invalid response received registering session\n");
  } catch (std::logic_error ex) {
    socket_->close();
    io_socket_->close();
    // printf("Error in registration response: %s\n", ex.what());
    throw std::runtime_error("Error in registration response\n");
  }

  if (response.getHeader().length != reg_data->getLength()) {
    // printf(
    //    "Registration message received with wrong size. Expected %zu bytes, "
    //    "received %u\n",
    //    reg_data->getLength(), response.getHeader().length);
  }

  bool response_valid = false;
  try {
    response.getPayloadAs(*reg_data);
    response_valid = true;
  } catch (std::length_error ex) {
    // printf("Registration message too short, ignoring\n");
  } catch (std::logic_error ex) {
    // printf("Could not parse registration response: %s\n", ex.what());
  }

  if (response_valid && reg_data->protocol_version != EIP_PROTOCOL_VERSION) {
    // printf(
    //    "Error: Wrong Ethernet Industrial Protocol Version. Expected %u got "
    //    "%u\n",
    //   EIP_PROTOCOL_VERSION, reg_data->protocol_version);
    socket_->close();
    io_socket_->close();
    throw std::runtime_error(
        "Received wrong Ethernet IP Protocol Version on registration\n");
  }
  if (response_valid && reg_data->options != 0) {
    // printf("Registration message included non-zero options flags: %u\n",
    //       reg_data->options);
  }

  session_id_ = response.getHeader().session_handle;
  // printf("Successfully opened session ID %zu\n", session_id_);
}

void Session::close() {
  // TODO: should close all connections and the IO port
  // printf("Closing session\n");

  // create the unregister session message
  EncapPacket reg_msg(EIP_CMD_UNREGISTER_SESSION, session_id_);
  socket_->send(reg_msg);

  // printf("Session closed\n");

  socket_->close();
  io_socket_->close();
  session_id_ = 0;
}

EncapPacket Session::sendCommand(EncapPacket& req) {
  // printf("Sending Command\n");
  socket_->send(req);

  // printf("Waiting for response\n");
  size_t n = socket_->receive(buffer(recv_buffer_));
  // printf("Received response of %zu bytes\n", n);

  BufferReader reader(buffer(recv_buffer_, n));
  EncapPacket result;
  result.deserialize(reader);

  if (reader.getByteCount() != n) {
    // printf("Packet received with %zu bytes, but only %zu bytes used\n", n,
    //       reader.getByteCount());
  }

  check_packet(result, req.getHeader().command);
  return result;
}

void Session::check_packet(EncapPacket& pkt, EIP_UINT exp_cmd) {
  // verify that all fields are correct
  if (pkt.getHeader().command != exp_cmd) {
    // printf("Reply received with wrong command. Expected %u received %u\n",
    //       exp_cmd, pkt.getHeader().command);
    throw std::logic_error("Reply received with wrong command\n");
  }
  if (session_id_ == 0 && pkt.getHeader().session_handle == 0) {
    // printf("Zero session handle received on registration: %zu\n",
    //      pkt.getHeader().session_handle);
    throw std::logic_error("Zero session handle received on registration\n");
  }
  if (session_id_ != 0 && pkt.getHeader().session_handle != session_id_) {
    // printf("Reply received with wrong session ID. Expected %zu, received
    // %zu\n",
    //       session_id_, pkt.getHeader().session_handle);
    throw std::logic_error("Wrong session ID received for command\n");
  }
  if (pkt.getHeader().status != 0) {
    // printf("Non-zero status received: %zu\n", pkt.getHeader().status);
  }
  if (pkt.getHeader().context[0] != 0 || pkt.getHeader().context[1] != 0) {
    // printf("Non-zero sender context received: %zu/%zu\n",
    //      pkt.getHeader().context[0], pkt.getHeader().context[1]);
  }
  if (pkt.getHeader().options != 0) {
    // printf("Non-zero options received: %zu\n", pkt.getHeader().options);
  }
}

void Session::getSingleAttributeSerializable(EIP_USINT class_id,
                                             EIP_USINT instance_id,
                                             EIP_USINT attribute_id,
                                             Serializable& result) {
  shared_ptr<Serializable> no_data;
  RRDataResponse resp_data = sendRRDataCommand(
      0x0E, Path(class_id, instance_id, attribute_id), no_data);

  resp_data.getResponseDataAs(result);
}

void Session::setSingleAttributeSerializable(EIP_USINT class_id,
                                             EIP_USINT instance_id,
                                             EIP_USINT attribute_id,
                                             shared_ptr<Serializable> data) {
  RRDataResponse resp_data =
      sendRRDataCommand(0x10, Path(class_id, instance_id, attribute_id), data);
}

RRDataResponse Session::sendRRDataCommand(EIP_USINT service, const Path& path,
                                          shared_ptr<Serializable> data) {
  // printf("Creating RR Data Request\n");
  shared_ptr<RRDataRequest> req_data =
      make_shared<RRDataRequest>(service, path, data);
  EncapPacket encap_pkt(EIP_CMD_SEND_RR_DATA, session_id_, req_data);

  // send command and get response
  EncapPacket response;
  try {
    response = sendCommand(encap_pkt);
  } catch (std::length_error ex) {
    // printf("Response packet to RR command too short: %s\n", ex.what());
    throw std::runtime_error("Packet response to RR Data Command too short\n");
  } catch (std::logic_error ex) {
    // printf("Invalid response to RR command: %s\n", ex.what());
    throw std::runtime_error("Invalid packet response to RR Data Command\n");
  }

  RRDataResponse resp_data;
  try {
    response.getPayloadAs(resp_data);
  } catch (std::length_error ex) {
    // printf("Response data to RR command too short: %s\n", ex.what());
    throw std::runtime_error("Response data to RR Command too short\n");
  } catch (std::logic_error ex) {
    // printf("Invalid data to RR command: %s\n", ex.what());
    throw std::runtime_error("Invalid data in response to RR command\n");
  }

  // check that responses are valid
  if (resp_data.getServiceCode() != (service | 0x80)) {
    // printf(
    //    "Wrong service code returned for RR Data command. Expected: %d but "
    //    "received %d\n",
    //    (int)service, (int)resp_data.getServiceCode());
    // throw std::runtime_error("Wrong service code returned for RR Data
    // command");
  }
  if (resp_data.getGeneralStatus()) {
    // printf("RR Data Command failed with status %d\n",
    //       (int)resp_data.getGeneralStatus());
    throw std::runtime_error("RR Data Command Failed\n");
  }
  return resp_data;
}

int Session::createConnection(const EIP_CONNECTION_INFO_T& o_to_t,
                              const EIP_CONNECTION_INFO_T& t_to_o) {
  Connection conn(o_to_t, t_to_o);
  conn.originator_vendor_id = my_vendor_id_;
  conn.originator_sn = my_serial_num_;
  conn.connection_sn = next_connection_sn_++;
  conn.o_to_t_connection_id = next_connection_id_++;
  conn.t_to_o_connection_id = next_connection_id_++;

  shared_ptr<ForwardOpenRequest> req = conn.createForwardOpenRequest();
  RRDataResponse resp_data = sendRRDataCommand(0x5B, Path(0x06, 1), req);
  ForwardOpenSuccess result;
  resp_data.getResponseDataAs(result);
  if (!conn.verifyForwardOpenResult(result)) {
    // printf("Received invalid response to forward open request\n");
    throw std::logic_error("Forward Open Response Invalid\n");
  }

  connections_.push_back(conn);
  return connections_.size() - 1;
}

void Session::closeConnection(size_t n) {
  shared_ptr<ForwardCloseRequest> req =
      connections_[n].createForwardCloseRequest();
  RRDataResponse resp_data = sendRRDataCommand(0x4E, Path(0x06, 1), req);
  ForwardCloseSuccess result;
  resp_data.getResponseDataAs(result);
  if (!connections_[n].verifyForwardCloseResult(result)) {
    // printf("Received invalid response to forward close request\n");
    throw std::logic_error("Forward Close Response Invalid\n");
  }
  // remove the connection from the list
  connections_.erase(connections_.begin() + n);
}

CPFPacket Session::receiveIOPacket() {
  // printf("Receiving IO packet\n");
  size_t n = io_socket_->receive(buffer(recv_buffer_));
  // printf("Received IO of %zu bytes\n", n);

  BufferReader reader(buffer(recv_buffer_, n));
  CPFPacket result;
  result.deserialize(reader);

  if (reader.getByteCount() != n) {
    // printf("IO packet received with %zu bytes, but only %zu bytes used\n", n,
    //       reader.getByteCount());
  }

  return result;
}

void Session::sendIOPacket(CPFPacket& pkt) {
  // printf("Sending CPF Packet on IO Socket\n");
  io_socket_->send(pkt);
}

}  // namespace eip
