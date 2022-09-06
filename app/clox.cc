#include <iostream>
#include <memory>
#include <span>

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

std::shared_ptr<clox::Memory>
interpret(const std::string& content, clox::VM& vm, std::shared_ptr<clox::Memory> memory)
{
  using namespace clox;

  auto r = boost::leaf::try_handle_some(
    [&]() -> boost::leaf::result<std::shared_ptr<Memory>>
    {
      BOOST_LEAF_AUTO(chunk, Compile{Scanner{content}}(std::move(memory)));
      const auto result = vm(std::move(chunk));

      return result.memory;
    },
    [](std::shared_ptr<Memory> new_memory, const std::string& error_msg)
    {
      std::cerr << error_msg << '\n';
      return new_memory;
    });

  return r.value();
}

std::shared_ptr<clox::Memory>
interpret(const std::string& content)
{
  auto vm = clox::VM{clox::VM::opt_disassemble::yes};
  auto code_context = std::make_shared<clox::Memory>();
  return interpret(content, vm, std::move(code_context));
}

void
repl()
{
  auto vm = clox::VM{clox::VM::opt_disassemble::no};
  auto memory = std::make_shared<clox::Memory>();

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
main(int argc, char** _argv)
{
  using namespace clox;
  try
  {
    const auto argv = std::span{_argv, static_cast<std::size_t>(argc)};

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
  catch (const std::exception& e)
  {
    std::cerr << e.what() << '\n';
    return -1;
  }
}

// ---------------------------------------------------------------------------------------------- //
