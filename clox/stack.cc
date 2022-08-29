#include "clox/stack.hh"

namespace clox {

// ---------------------------------------------------------------------------------------------- //

Stack::Stack()
  : stack_{}
{
  stack_.reserve(1024);
}

// ---------------------------------------------------------------------------------------------- //

void
Stack::push(Value value)
{
  stack_.push_back(value);
}

// ---------------------------------------------------------------------------------------------- //

Value
Stack::pop()
{
  const auto value = stack_.back();
  stack_.pop_back();
  return value;
}

// ---------------------------------------------------------------------------------------------- //

Value
Stack::peek(int distance) const
{
  return *(cend(stack_) - 1 - distance);
}

// ---------------------------------------------------------------------------------------------- //

const Value&
Stack::top() const
{
  return stack_.back();
}

Value&
Stack::top()
{
  return stack_.back();
}

// ---------------------------------------------------------------------------------------------- //

std::size_t
Stack::size() const noexcept
{
  return stack_.size();
}

// ---------------------------------------------------------------------------------------------- //

} // clox
