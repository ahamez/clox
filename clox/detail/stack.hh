#pragma once

#include <vector>

#include "clox/value.hh"

namespace clox::detail {

// ---------------------------------------------------------------------------------------------- //

class Stack
{
  static constexpr auto initial_stack_size = 1024;

public:
  Stack() { stack_.reserve(initial_stack_size); }

  void push(Value value) { stack_.push_back(value); }

  [[nodiscard]] Value pop()
  {
    const auto value = stack_.back();
    stack_.pop_back();
    return value;
  }

  [[nodiscard]] const Value& top() const { return stack_.back(); }
  [[nodiscard]] Value& top() { return stack_.back(); }

private:
  std::vector<Value> stack_{};
};

// ---------------------------------------------------------------------------------------------- //

} // namespace clox::detail
