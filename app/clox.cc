#include <iostream>
#include <ranges>
#include <string>

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

void
interpret(const std::string& content, clox::VM::opt_disassemble disassemble)
{
  using namespace clox;
  auto scanner = Scanner{content};
  if (const auto chunk = Compile{std::move(scanner)}(); chunk)
  {
    VM{disassemble}(chunk.get());
  }
  else
  {
    std::cerr << chunk.error() << '\n';
  }
}

void
repl()
{
  std::cout << "> ";
  for (auto line = std::string{}; std::getline(std::cin, line);)
  {
    interpret(line, clox::VM::opt_disassemble::no);
    std::cout << "> ";
  }
  std::cout << "Good bye!\n";
}
} // namespace anonymous

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
    interpret(file_content, VM::opt_disassemble::yes);
  }
  else
  {
    std::cerr << "Usage: clox [path]\n";
    return 64;
  }

  return 0;
}

// ---------------------------------------------------------------------------------------------- //
