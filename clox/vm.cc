#include <iostream>

#include "clox/detail/interpret.hh"
#include "clox/detail/stack.hh"
#include "clox/disassemble.hh"
#include "clox/vm.hh"

namespace clox {
namespace /* anonymous */ {

constexpr auto globals_reserve = 1024;

// ---------------------------------------------------------------------------------------------- //

template<VM::opt_disassemble Disassemble>
[[nodiscard]] InterpretResult
run(Chunk& chunk, VM& vm)
{
  auto current_ip = chunk.code->cbegin();
  auto stack = detail::Stack{};

  try
  {
    while (true)
    {
      if constexpr (Disassemble == VM::opt_disassemble::yes)
      {
        std::cout << disassemble(current_ip, chunk) << '\n';
      }
      current_ip = detail::Interpret{chunk, vm, stack, current_ip}.visit();
    }
  }
  catch (const detail::InterpretReturn& r)
  {
    if (r.status != InterpretResultStatus::ok)
    {
      std::cerr << "line " << chunk.code->line(current_ip).value_or(0) << ": " << r.message << '\n';
    }
    return {r.status, std::move(chunk.memory)};
  }
}

} // namespace

// ---------------------------------------------------------------------------------------------- //

VM::VM(VM::opt_disassemble disassemble)
  : disassemble_{disassemble}
{
  globals_.reserve(globals_reserve);
}

// ---------------------------------------------------------------------------------------------- //

InterpretResult
VM::operator()(Chunk&& chunk)
{
  if (disassemble_ == opt_disassemble::yes)
  {
    return run<opt_disassemble::yes>(chunk, *this);
  }
  else
  {
    return run<opt_disassemble::no>(chunk, *this);
  }
}

// ---------------------------------------------------------------------------------------------- //

} // namespace clox
