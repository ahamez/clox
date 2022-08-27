#pragma once

#include "clox/chunk.hh"

namespace clox {

// ---------------------------------------------------------------------------------------------- //

enum class InterpretResult
{
  ok,
  compile_error,
  runtime_error
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

  [[nodiscard]] InterpretResult operator()(Chunk&) const;

private:
  opt_disassemble disassemble_;
};

// ---------------------------------------------------------------------------------------------- //

}
