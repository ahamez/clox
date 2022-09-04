#pragma once

#include <iosfwd>
#include <optional>
#include <unordered_map>
#include <vector>

#include "clox/chunk_context.hh"
#include "clox/chunk_fwd.hh"
#include "clox/memory.hh"
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

  [[nodiscard]] ConstantIndex add_constant(Value);
  [[nodiscard]] Value get_constant(ConstantIndex) const;

  [[nodiscard]] std::string disassemble(code_const_iterator current_opcode,
                                        const ChunkContext&) const;

  [[nodiscard]] code_const_iterator code_cbegin() const noexcept;
  [[nodiscard]] std::size_t code_offset(code_const_iterator code_cit) const;
  [[nodiscard]] std::optional<std::size_t> line(code_const_iterator code_cit) const;

  [[nodiscard]] Memory& memory() noexcept;

private:
  std::vector<Opcode> code_{};
  std::vector<std::optional<std::size_t>> lines_{};
  std::vector<Value> constants_{};
  Memory memory_{};
};

// ---------------------------------------------------------------------------------------------- //

} // namespace clox
