#pragma once

#include <cassert>
#include <cstdint>
#include <optional>
#include <ostream>
#include <vector>

#include <fmt/core.h>

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

  std::size_t add_value(Value v)
  {
    const auto it = values_.insert(values_.end(), v);
    return it - cbegin(values_);
  }

  [[nodiscard]] Value get_value(std::size_t offset) const { return values_[offset]; }

  friend std::ostream& operator<<(std::ostream& os, const Chunk& chunk)
  {
    for (auto code_cit = cbegin(chunk.code_); code_cit != cend(chunk.code_); ++code_cit)
    {
      os << chunk.disassemble(code_cit) << '\n';
    }

    return os;
  }

  [[nodiscard]] std::string disassemble(code_const_iterator current_opcode) const
  {
    assert(code_.size() == lines_.size());

    const auto offset = current_opcode - cbegin(code_);
    const auto lines_cit = cbegin(lines_) + offset;

    return fmt::format("{:04d} | {:04d} | {}",
                       offset,
                       lines_cit->value_or(0),
                       disassemble_opcode(*current_opcode, *this));
  }

  [[nodiscard]] code_const_iterator code_cbegin() const noexcept { return cbegin(code_); }

  [[nodiscard]] auto code_offset(code_const_iterator code_cit) const
  {
    return std::distance(cbegin(code_), code_cit);
  }

  [[nodiscard]] auto line(code_const_iterator code_cit) const
  {
    return lines_[code_offset(code_cit)];
  }

private:
  // The three following vectors always have the same size.
  std::vector<Opcode> code_;
  std::vector<Value> values_;
  std::vector<std::optional<std::size_t>> lines_;
};

// ---------------------------------------------------------------------------------------------- //

} // clox