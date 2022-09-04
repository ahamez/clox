#pragma once

#include "clox/code.hh"
#include "clox/code_context.hh"

namespace clox {

// ---------------------------------------------------------------------------------------------- //

struct Chunk
{
  Code& code;
  CodeContext& code_cxt;
};

// ---------------------------------------------------------------------------------------------- //

} // namespace clox
