#pragma once

#include "clox/memory.hh"

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
  std::unique_ptr<Memory> memory{};
};

// ---------------------------------------------------------------------------------------------- //

} // namespace clox
