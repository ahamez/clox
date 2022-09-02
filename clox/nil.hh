#pragma once

#include <iosfwd>

namespace clox {

// ---------------------------------------------------------------------------------------------- //

struct Nil
{
  friend std::ostream& operator<<(std::ostream& os, const Nil&) { return os << "nil"; }
};

// ---------------------------------------------------------------------------------------------- //

} // namespace clox
