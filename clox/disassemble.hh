#pragma once

#include "clox/chunk.hh"

namespace clox {

// ---------------------------------------------------------------------------------------------- //

[[nodiscard]] std::string
disassemble(Code::const_iterator, const Chunk&);

// ---------------------------------------------------------------------------------------------- //

} // namespace clox
