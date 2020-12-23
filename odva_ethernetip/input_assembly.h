/**
Software License Agreement
\file      input_assembly.h
\authors   Bill McCormick <wmccormick@swri.org>
\copyright
*/

#ifndef INPUT_ASSEMBLY_H
#define INPUT_ASSEMBLY_H

#include "odva_ethernetip/eip_types.h"
#include "odva_ethernetip/serialization/reader.h"
#include "odva_ethernetip/serialization/serializable.h"
#include "odva_ethernetip/serialization/writer.h"

using eip::serialization::Reader;
using eip::serialization::Serializable;
using eip::serialization::Writer;

/**
 * Data structure and operators.
 */
class InputAssembly : public Serializable {
 public:
  EIP_UINT sensor_and_control_port_inputs;
  EIP_UINT sensor_detect;
  EIP_UINT left_motor_temperature;
  EIP_UINT left_motor_status;
  EIP_UINT right_motor_temperature;
  EIP_UINT right_motor_status;
  EIP_UINT left_motor_port_digital_io_status;
  EIP_UINT right_motor_port_digital_io_status;
  EIP_UINT reserved;

  /**
   * Size of this message including all data
   */
  virtual size_t getLength() const { return 54; }
  /**
   * Serialize data into the given buffer
   * @param writer Writer to use for serialization
   * @return the writer again
   * @throw std::length_error if the buffer is too small for the header data
   */
  virtual Writer& serialize(Writer& writer) const {
    writer.write(sensor_and_control_port_inputs);
    writer.write(sensor_detect);
    writer.write(left_motor_temperature);
    writer.write(left_motor_status);
    writer.write(right_motor_temperature);
    writer.write(right_motor_status);
    writer.write(left_motor_port_digital_io_status);
    writer.write(right_motor_port_digital_io_status);
    writer.write(reserved);
    return writer;
  }

  /**
   * Extra length information is not relevant in this context. Same as
   * deserialize(reader)
   */
  virtual Reader& deserialize(Reader& reader, size_t length) {
    deserialize(reader);
    return reader;
  }

  /**
   * Deserialize data from the given reader without length information
   * @param reader Reader to use for deserialization
   * @return the reader again
   * @throw std::length_error if the buffer is overrun while deserializing
   */
  virtual Reader& deserialize(Reader& reader) {
    reader.read(sensor_and_control_port_inputs);
    reader.read(sensor_detect);
    reader.read(left_motor_temperature);
    reader.read(left_motor_status);
    reader.read(right_motor_temperature);
    reader.read(right_motor_status);
    reader.read(left_motor_port_digital_io_status);
    reader.read(right_motor_port_digital_io_status);
    reader.read(reserved);
    return reader;
  }
};

#endif  // INPUT_ASSEMBLY_H
