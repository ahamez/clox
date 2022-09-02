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
  [[nodiscard]] const Value& top() const;
  [[nodiscard]] Value& top();
  [[nodiscard]] std::size_t size() const noexcept;

private:
  std::vector<Value> stack_{};
};

// ---------------------------------------------------------------------------------------------- //

} // namespace clox
