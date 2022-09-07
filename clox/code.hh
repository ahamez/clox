#pragma once

#include <iosfwd>
#include <optional>
#include <unordered_map>
#include <vector>

#include "clox/detail/index.hh"
#include "clox/opcode.hh"
#include "clox/value.hh"

namespace clox {

// ---------------------------------------------------------------------------------------------- //

class Code
{
public:
  using const_iterator = std::vector<Opcode>::const_iterator;

public:
  template<typename Opcode>
  void add_opcode(Opcode&& op, std::optional<std::size_t> line = {})
  {
    code_.emplace_back(std::forward<Opcode>(op));
    lines_.emplace_back(line);
  }

  [[nodiscard]] detail::ConstantIndex add_constant(Value);
  [[nodiscard]] Value get_constant(detail::ConstantIndex) const;

  [[nodiscard]] const_iterator cbegin() const noexcept;

  [[nodiscard]] std::size_t code_offset(const_iterator code_cit) const;
  [[nodiscard]] std::optional<std::size_t> line(const_iterator code_cit) const;

private:
  std::vector<Opcode> code_{};
  std::vector<std::optional<std::size_t>> lines_{};
  std::vector<Value> constants_{};
};

// ---------------------------------------------------------------------------------------------- //

} // namespace clox
