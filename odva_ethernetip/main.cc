#include <iostream>

#include "odva_ethernetip/input_assembly.h"
#include "odva_ethernetip/io_scanner.h"
#include "odva_ethernetip/output_assembly.h"
#include "odva_ethernetip/session.h"
#include "odva_ethernetip/socket/tcp_socket.h"
#include "odva_ethernetip/socket/udp_socket.h"

int main(int args, char** argv) {
  using eip::socket::TCPSocket;
  using eip::socket::UDPSocket;

  std::string host(argv[1]);

  boost::asio::io_service boost_io = {};

  boost::asio::io_service io_service;
  boost::shared_ptr<TCPSocket> socket =
      shared_ptr<TCPSocket>(new TCPSocket(io_service));
  boost::shared_ptr<UDPSocket> io_socket =
      shared_ptr<UDPSocket>(new UDPSocket(io_service, 2222));
  eip::Session s(socket, io_socket);

  boost::shared_ptr<TCPSocket> socket1 =
      shared_ptr<TCPSocket>(new TCPSocket(io_service));
  boost::shared_ptr<UDPSocket> io_socket1 =
      shared_ptr<UDPSocket>(new UDPSocket(io_service, 2222));
  eip::Session s1(socket1, io_socket1);

  try {
    s.open(argv[1]);
    s1.open(argv[2]);

    InputAssembly ia;
    s.getSingleAttributeSerializable(0x04, 107, 3, ia);

    std::cout << "Input Assembly: " << std::endl;
    std::cout << ia.sensor_and_control_port_inputs << std::endl;
    std::cout << ia.sensor_detect << std::endl;

    s1.getSingleAttributeSerializable(0x04, 107, 3, ia);

    std::cout << "Input Assembly: " << std::endl;
    std::cout << ia.sensor_and_control_port_inputs << std::endl;
    std::cout << ia.sensor_detect << std::endl;

  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }

  // eip::IOScanner i(boost_io, host);
  // i.run();

  s.close();
  s1.close();

  return 0;
}