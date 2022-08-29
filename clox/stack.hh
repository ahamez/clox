#pragma once

#include <vector>

#include "clox/value.hh"

namespace clox {

// ---------------------------------------------------------------------------------------------- //

class Stack
{
public:
  Stack();

  void push(Value value);
  [[nodiscard]] Value pop();
  [[nodiscard]] Value peek(int distance) const;
  [[nodiscard]] const Value& last() const;
  [[nodiscard]] Value& last();

private:
  std::vector<Value> stack_;
};

// ---------------------------------------------------------------------------------------------- //

} // clox
