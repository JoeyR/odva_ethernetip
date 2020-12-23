/**
Software License Agreement
\file      input_assembly.h
\authors   Bill McCormick <wmccormick@swri.org>
\copyright
*/

#ifndef OUTPUT_ASSEMBLY_H
#define OUTPUT_ASSEMBLY_H

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
class OutputAssembly : public Serializable {
 public:
  EIP_UINT set_left_motor_port_digital_control;
  EIP_UINT set_right_motor_port_digital_control;
  EIP_UINT control_port_digital_output_control;
  EIP_UINT left_motor_run_or_reverse;
  EIP_UINT right_motor_run_or_reverse;
  EIP_UINT left_motor_speed_reference;
  EIP_UINT right_motor_speed_reference;
  EIP_UINT clear_motor_error;
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
    writer.write(set_left_motor_port_digital_control);
    writer.write(set_right_motor_port_digital_control);
    writer.write(control_port_digital_output_control);
    writer.write(left_motor_run_or_reverse);
    writer.write(right_motor_run_or_reverse);
    writer.write(left_motor_speed_reference);
    writer.write(right_motor_speed_reference);
    writer.write(clear_motor_error);
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
    reader.read(set_left_motor_port_digital_control);
    reader.read(set_right_motor_port_digital_control);
    reader.read(control_port_digital_output_control);
    reader.read(left_motor_run_or_reverse);
    reader.read(right_motor_run_or_reverse);
    reader.read(left_motor_speed_reference);
    reader.read(right_motor_speed_reference);
    reader.read(clear_motor_error);
    reader.read(reserved);
    return reader;
  }
};

#endif  // OUTPUT_ASSEMBLY_H
