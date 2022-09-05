#include <fmt/core.h>

#include "clox/disassemble.hh"

namespace clox {

// ---------------------------------------------------------------------------------------------- //

std::string
disassemble(Code::const_iterator opcode_cit, const Chunk& chunk)
{
  return fmt::format("{:04d} | {:04d} | {}",
                     chunk.code->code_offset(opcode_cit),
                     chunk.code->line(opcode_cit).value_or(0),
                     disassemble_opcode(*opcode_cit, chunk));
}

// ---------------------------------------------------------------------------------------------- //

} // namespace clox
