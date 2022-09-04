#pragma once

#include <cstdint>

namespace clox {

// ---------------------------------------------------------------------------------------------- //

struct ConstantIndex
{
  std::uint16_t index{};
};

// ---------------------------------------------------------------------------------------------- //

struct GlobalVariableIndex
{
  std::uint16_t index{};
};

// ---------------------------------------------------------------------------------------------- //

class Chunk;

// ---------------------------------------------------------------------------------------------- //

} // namespace clox
