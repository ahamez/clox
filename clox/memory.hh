#pragma once

#include <vector>

#include "clox/obj_string.hh"

namespace clox {

// ---------------------------------------------------------------------------------------------- //

class Memory
{
public:
  Memory()
    : string_set_buckets_{1024}
    , string_set_{ObjStringSet::bucket_traits{string_set_buckets_.data(), 1024}}
  {}
  //  TODO: check if move constructor is valid with an intrusive container
  Memory(Memory&&) = default;
  ~Memory();

  [[nodiscard]] const ObjString* make_string(std::string);

private:
  std::vector<ObjStringSet::bucket_type> string_set_buckets_;
  ObjStringSet string_set_;
};

// ---------------------------------------------------------------------------------------------- //

} // clox
