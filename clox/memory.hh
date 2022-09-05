#pragma once

#include <unordered_map>
#include <vector>

#include "clox/code_fwd.hh"
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
  //  TODO: check if move constructor is valid with an intrusive container
  Memory(Memory&&) = default;
  ~Memory();

  [[nodiscard]] const ObjString* make_string(std::string);

  [[nodiscard]] GlobalVariableIndex maybe_add_global_variable(const std::string&);
  [[nodiscard]] std::string get_global_variable(GlobalVariableIndex) const;
  [[nodiscard]] GlobalVariableIndex last_global_variable_index() const noexcept;

private:
  std::vector<ObjStringSet::bucket_type> string_set_buckets_;
  ObjStringSet string_set_;

  std::unordered_map<std::string, GlobalVariableIndex> global_variables_{};
  GlobalVariableIndex last_global_variable_index_{0};
};

// ---------------------------------------------------------------------------------------------- //

} // namespace clox
