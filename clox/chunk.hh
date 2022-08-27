#pragma once

#include <cstdint>
#include <iosfwd>
#include <optional>
#include <vector>

#include "clox/opcode.hh"
#include "clox/value.hh"

namespace clox {

// ---------------------------------------------------------------------------------------------- //

class Chunk
{
public:
  using code_const_iterator = std::vector<Opcode>::const_iterator;

public:
  template<typename Opcode>
  void add_opcode(Opcode&& op, std::optional<std::size_t> line = {})
  {
    code_.emplace_back(std::forward<Opcode>(op));
    lines_.emplace_back(line);
  }

  [[nodiscard]] std::uint16_t add_value(Value);
  [[nodiscard]] Value get_value(std::size_t offset) const;
  [[nodiscard]] std::string disassemble(code_const_iterator current_opcode) const;
  [[nodiscard]] code_const_iterator code_cbegin() const noexcept;
  [[nodiscard]] std::size_t code_offset(code_const_iterator code_cit) const;
  [[nodiscard]] std::optional<std::size_t> line(code_const_iterator code_cit) const;

  friend std::ostream& operator<<(std::ostream& os, const Chunk& chunk);

private:
  std::vector<Opcode> code_;
  std::vector<std::optional<std::size_t>> lines_;
  std::vector<Value> values_;
};

// ---------------------------------------------------------------------------------------------- //

} // clox