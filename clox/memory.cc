#include "clox/memory.hh"

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

} // namespace clox
