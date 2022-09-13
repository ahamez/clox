#pragma once

#include <unordered_map>
#include <vector>

#include "clox/detail/index.hh"
#include "clox/obj_string.hh"

namespace clox {

// ---------------------------------------------------------------------------------------------- //

class Memory
{
  static constexpr auto string_set_nb_buckets = 1024;

public:
  Memory()
    : string_set_buckets_{string_set_nb_buckets}
    , string_set_{ObjStringSet::bucket_traits{string_set_buckets_.data(), string_set_nb_buckets}}
  {}

  ~Memory();
  //  TODO: check if move constructor is valid with an intrusive container
  Memory(const Memory&) = delete;
  Memory(Memory&&) noexcept = default;
  Memory& operator=(const Memory&) = delete;
  Memory& operator=(Memory&&) = default;

  [[nodiscard]] const ObjString* make_string(std::string);

  [[nodiscard]] detail::GlobalVariableIndex maybe_add_global_variable(const std::string&);
  [[nodiscard]] std::string get_global_variable(detail::GlobalVariableIndex) const;

private:
  std::vector<ObjStringSet::bucket_type> string_set_buckets_;
  ObjStringSet string_set_;

  std::unordered_map<std::string, detail::GlobalVariableIndex> global_variables_{};
  detail::GlobalVariableIndex last_global_variable_index_{0};
};

// ---------------------------------------------------------------------------------------------- //

} // namespace clox
