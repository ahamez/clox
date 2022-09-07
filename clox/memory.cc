#include "clox/memory.hh"

#include <fmt/core.h>

namespace clox {

// ---------------------------------------------------------------------------------------------- //

Memory::~Memory()
{
  string_set_.clear_and_dispose([](auto obj) { delete obj; });
}

// ---------------------------------------------------------------------------------------------- //

const ObjString*
Memory::make_string(std::string str)
{
  // TODO: rehash if string_set_.size() >= 1024. Requires constant-time size().

  auto commit = ObjStringSet::insert_commit_data{};
  const auto [it, inserted] = string_set_.insert_check(
    str,
    [](auto&& lhs) { return std::hash<std::string>{}(lhs); },
    [](auto&& lhs, auto&& rhs) { return lhs == rhs.str; },
    commit);

  if (inserted)
  {
    auto* obj = new ObjString{std::move(str)};
    string_set_.insert_commit(*obj, commit);
    return obj;
  }
  else
  {
    return &*it;
  }
}

// ---------------------------------------------------------------------------------------------- //

detail::GlobalVariableIndex
Memory::maybe_add_global_variable(const std::string& name)
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
Memory::get_global_variable(clox::detail::GlobalVariableIndex index) const
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

detail::GlobalVariableIndex
Memory::last_global_variable_index() const noexcept
{
  return last_global_variable_index_;
}

// ---------------------------------------------------------------------------------------------- //

} // namespace clox
