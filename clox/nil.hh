#pragma once

#include <ostream>

namespace clox {

// ---------------------------------------------------------------------------------------------- //

struct Nil
{
  friend std::ostream& operator<<(std::ostream& os, const Nil&) { return os << "nil"; }
};

// ---------------------------------------------------------------------------------------------- //

}
