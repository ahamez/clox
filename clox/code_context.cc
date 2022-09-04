#include <algorithm>

#include <fmt/core.h>

#include "clox/code_context.hh"

namespace clox {

// ---------------------------------------------------------------------------------------------- //

GlobalVariableIndex
CodeContext::maybe_add_global_variable(const std::string& name)
{
  if (const auto search = global_variables_.find(name); search != cend(global_variables_))
  {
    return search->second;
  }
  else
  {
    const auto it = global_variables_.insert(search, {name, last_global_variable_index_});
    ++last_global_variable_index_;

    return it->second;
  }
}

std::string
CodeContext::get_global_variable(clox::GlobalVariableIndex index) const
{
  // TODO implement reverse lookup index -> variable name
  const auto search = std::find_if(cbegin(global_variables_),
                                   cend(global_variables_),
                                   [index](auto&& entry) { return entry.second == index; });

  if (search != std::cend(global_variables_))
  {
    return search->first;
  }
  else
  {
    throw std::runtime_error{
      fmt::format("Variable with index {} not found ", static_cast<std::uint16_t>(index))};
  }
}

GlobalVariableIndex
CodeContext::last_global_variable_index() const noexcept
{
  return last_global_variable_index_;
}

// ---------------------------------------------------------------------------------------------- //

} // namespace clox
