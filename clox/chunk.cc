#include <cassert>

#include <fmt/core.h>

#include "clox/chunk.hh"

namespace clox {

// ---------------------------------------------------------------------------------------------- //

std::uint16_t
Chunk::add_value(Value v)
{
  const auto it = values_.insert(values_.end(), v);
  return it - cbegin(values_);
}

// ---------------------------------------------------------------------------------------------- //

Value
Chunk::get_value(std::size_t offset) const
{
  return values_[offset];
}

// ---------------------------------------------------------------------------------------------- //

std::string
Chunk::disassemble(Chunk::code_const_iterator current_opcode) const
{
  assert(code_.size() == lines_.size());

  const auto offset = current_opcode - cbegin(code_);
  const auto lines_cit = cbegin(lines_) + offset;

  return fmt::format("{:04d} | {:04d} | {}",
                     offset,
                     lines_cit->value_or(0),
                     disassemble_opcode(*current_opcode, *this));
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

std::ostream&
operator<<(std::ostream& os, const Chunk& chunk)
{
  for (auto code_cit = cbegin(chunk.code_); code_cit != cend(chunk.code_); ++code_cit)
  {
    os << chunk.disassemble(code_cit) << '\n';
  }

  return os;
}

// ---------------------------------------------------------------------------------------------- //

} // namespace clox
