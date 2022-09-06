#pragma once

#include <memory>

#include "clox/code.hh"
#include "clox/memory.hh"

namespace clox {

// ---------------------------------------------------------------------------------------------- //

struct Chunk
{
  std::shared_ptr<Code> code;
  std::shared_ptr<Memory> memory;
};

// ---------------------------------------------------------------------------------------------- //

} // namespace clox
