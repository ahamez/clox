#pragma once

namespace clox::detail {

// ---------------------------------------------------------------------------------------------- //

template<class... Ts>
struct visitor : Ts...
{
  using Ts::operator()...;
};
template<class... Ts>
visitor(Ts...) -> visitor<Ts...>;

// ---------------------------------------------------------------------------------------------- //

} // namespace clox::detail
