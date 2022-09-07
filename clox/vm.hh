#pragma once

#include <memory>
#include <unordered_map>

#include "clox/chunk.hh"
#include "clox/detail/index.hh"
#include "clox/memory.hh"
#include "clox/stack.hh"

namespace clox {

// ---------------------------------------------------------------------------------------------- //

enum class InterpretResultStatus
{
  ok,
  runtime_error
};

// ---------------------------------------------------------------------------------------------- //

struct InterpretResult
{
  InterpretResultStatus status{};
  Stack stack{};
  std::unique_ptr<Memory> memory{};
};

// ---------------------------------------------------------------------------------------------- //

class VM
{
public:
  enum class opt_disassemble
  {
    yes,
    no
  };

  explicit VM(opt_disassemble disassemble = opt_disassemble::no);

  [[nodiscard]] InterpretResult operator()(Chunk&);
  [[nodiscard]] InterpretResult operator()(Chunk&&);

  [[nodiscard]] auto& globals() noexcept { return globals_; }

private:
  opt_disassemble disassemble_{opt_disassemble::no};
  std::unordered_map<detail::GlobalVariableIndex, Value> globals_{};
};

// ---------------------------------------------------------------------------------------------- //

} // namespace clox
