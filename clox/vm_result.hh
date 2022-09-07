#pragma once

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

} // namespace clox
