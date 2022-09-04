#include <iterator>

#include "clox/code.hh"

namespace clox {

// ---------------------------------------------------------------------------------------------- //

ConstantIndex
Code::add_constant(Value v)
{
  const auto it = constants_.insert(constants_.end(), v);
  return {.index = static_cast<std::uint16_t>(it - constants_.cbegin())};
}

Value
Code::get_constant(ConstantIndex ref) const
{
  return constants_[ref.index];
}

// ---------------------------------------------------------------------------------------------- //

Code::const_iterator
Code::cbegin() const noexcept
{
  return code_.cbegin();
}

// ---------------------------------------------------------------------------------------------- //

std::size_t
Code::code_offset(Code::const_iterator code_cit) const
{
  return std::distance(code_.cbegin(), code_cit);
}

// ---------------------------------------------------------------------------------------------- //

std::optional<std::size_t>
Code::line(Code::const_iterator code_cit) const
{
  return lines_[code_offset(code_cit)];
}

// ---------------------------------------------------------------------------------------------- //

Memory&
Code::memory() noexcept
{
  return memory_;
}

// ---------------------------------------------------------------------------------------------- //

} // namespace clox
