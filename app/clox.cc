#include <iostream>
#include <memory>

#include "clox/compile.hh"
#include "clox/scanner.hh"
#include "clox/vm.hh"

#include "version.hh"

// ---------------------------------------------------------------------------------------------- //

namespace /* anonymous */
{
std::string
read_file(const std::string& file_path)
{
  auto file_stream = std::ifstream{file_path};
  auto file_content = std::string{};

  file_stream.seekg(0, std::ios::end);
  file_content.reserve(file_stream.tellg());
  file_stream.seekg(0, std::ios::beg);

  file_content.assign(std::istreambuf_iterator<char>{file_stream},
                      std::istreambuf_iterator<char>{});

  return file_content;
}

std::unique_ptr<clox::Memory>
interpret(const std::string& content, clox::VM& vm, std::unique_ptr<clox::Memory>&& memory)
{
  using namespace clox;

  auto maybe_chunk = Compile{Scanner{content}}(std::move(memory));

  if (maybe_chunk)
  {
    auto result = vm(std::move(maybe_chunk.get()));
    return std::move(result.memory);
  }
  else
  {
    return std::move(maybe_chunk.error());
  }
}

std::unique_ptr<clox::Memory>
interpret(const std::string& content)
{
  auto vm = clox::VM{clox::VM::opt_disassemble::yes};
  auto code_context = std::make_unique<clox::Memory>();
  return interpret(content, vm, std::move(code_context));
}

void
repl()
{
  auto vm = clox::VM{clox::VM::opt_disassemble::no};
  auto memory = std::make_unique<clox::Memory>();

  std::cout << "> ";
  for (auto line = std::string{}; std::getline(std::cin, line);)
  {
    memory = interpret(line, vm, std::move(memory));
    std::cout << "> ";
  }
  std::cout << "Good bye!\n";
}
} // namespace

// ---------------------------------------------------------------------------------------------- //

int
main(int argc, char** argv)
{
  using namespace clox;

  std::cout << "Clox interpreter (v" << CLOX_VERSION << ")\n";

  if (argc == 1)
  {
    repl();
  }
  else if (argc == 2)
  {
    const auto file_content = read_file(argv[1]);
    interpret(file_content);
  }
  else
  {
    std::cerr << "Usage: clox [path]\n";
    return -1;
  }

  return 0;
}

// ---------------------------------------------------------------------------------------------- //
