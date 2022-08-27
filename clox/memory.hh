#pragma once

#include "clox/obj_string.hh"

namespace clox {

// ---------------------------------------------------------------------------------------------- //

class Memory
{
public:
  Memory() = default;
  Memory(Memory&&) = default;
  ~Memory();

  template<typename... Args>
  [[nodiscard]] const ObjString* make_string(Args... args)
  {
    auto* obj = new ObjString{std::forward<Args>(args)...};
    string_list_.push_front(*obj);
    return obj;
  }

private:
  ObjStringList string_list_;
};

// ---------------------------------------------------------------------------------------------- //

} // clox
