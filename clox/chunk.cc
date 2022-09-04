#include <algorithm>
#include <cassert>

#include <fmt/core.h>

#include "clox/chunk.hh"

namespace clox {

// ---------------------------------------------------------------------------------------------- //

ConstantIndex
Chunk::add_constant(Value v)
{
  const auto it = constants_.insert(constants_.end(), v);
  return {.index = static_cast<std::uint16_t>(it - cbegin(constants_))};
}

Value
Chunk::get_constant(ConstantIndex ref) const
{
  return constants_[ref.index];
}

// ---------------------------------------------------------------------------------------------- //

std::string
Chunk::disassemble(Chunk::code_const_iterator current_opcode, const ChunkContext& chunk_cxt) const
{
  assert(code_.size() == lines_.size());

  const auto offset = current_opcode - cbegin(code_);
  const auto lines_cit = cbegin(lines_) + offset;

  return fmt::format("{:04d} | {:04d} | {}",
                     offset,
                     lines_cit->value_or(0),
                     disassemble_opcode(*current_opcode, *this, chunk_cxt));
}

// ---------------------------------------------------------------------------------------------- //

Chunk::code_const_iterator
Chunk::code_cbegin() const noexcept
{
  return cbegin(code_);
}

// ---------------------------------------------------------------------------------------------- //

std::size_t
Chunk::code_offset(Chunk::code_const_iterator code_cit) const
{
  return std::distance(cbegin(code_), code_cit);
}

// ---------------------------------------------------------------------------------------------- //

std::optional<std::size_t>
Chunk::line(Chunk::code_const_iterator code_cit) const
{
  return lines_[code_offset(code_cit)];
}

// ---------------------------------------------------------------------------------------------- //

Memory&
Chunk::memory() noexcept
{
  return memory_;
}

// ---------------------------------------------------------------------------------------------- //

} // namespace clox
