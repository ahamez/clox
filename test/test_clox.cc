#include <catch2/catch_test_macros.hpp>

#include "clox/compile.hh"
#include "clox/value.hh"
#include "clox/vm.hh"

// NOLINTBEGIN(readability-magic-numbers)

// auto
// interpret(const std::string& content)
//{
//   using namespace clox;
//
//   auto maybe_chunk = Compile{Scanner{content}}();
//   REQUIRE(static_cast<bool>(maybe_chunk) == true);
//   const auto result = VM{}(maybe_chunk.get());
//   REQUIRE(result.status == InterpretResultStatus::ok);
//   return result.stack.top();
// }

// TEST_CASE("Can interpret an arithmetic expression", "[vm]")
//{
//   REQUIRE(interpret("1+1;") == clox::Value{2.0});
// }
//
//  TEST_CASE("Can interpret a string comparison", "[vm]")
//{
//    static constexpr auto input = R"("a" + "b" == "ab;")";
//    REQUIRE(interpret(input) == clox::Value{true});
//  }

// NOLINTEND(readability-magic-numbers)
