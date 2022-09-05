#pragma once

#include <memory>

#include "clox/code.hh"
#include "clox/memory.hh"

namespace clox {

// ---------------------------------------------------------------------------------------------- //

struct Chunk
{
  std::unique_ptr<Code> code;
  std::unique_ptr<Memory> memory;
};

// ---------------------------------------------------------------------------------------------- //

} // namespace clox
