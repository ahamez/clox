#pragma once

#include <vector>

#include "clox/chunk.hh"
#include "clox/chunk_context.hh"
#include "clox/stack.hh"

namespace clox {

// ---------------------------------------------------------------------------------------------- //

enum class InterpretResultStatus
{
  ok,
  compile_error,
  runtime_error
};

// ---------------------------------------------------------------------------------------------- //

struct InterpretResult
{
  InterpretResultStatus status;
  Stack stack;
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

  VM(opt_disassemble disassemble = opt_disassemble::no);

  [[nodiscard]] InterpretResult operator()(Chunk&, ChunkContext&);

  auto& globals() noexcept { return globals_; }

private:
  opt_disassemble disassemble_{opt_disassemble::no};
  std::vector<Value> globals_{};
};

// ---------------------------------------------------------------------------------------------- //

} // namespace clox
