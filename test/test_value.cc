#include <catch2/catch_test_macros.hpp>

#include "clox/value.hh"

using namespace clox;

TEST_CASE("Can create a Value from a boolean", "[value]")
{
  const auto v = Value{true};
  REQUIRE(v.is<bool>());
  REQUIRE(not v.is<double>());
  REQUIRE(v.as<bool>() == true);
}

TEST_CASE("Can create a Value from a double", "[value]")
{
  const auto v = Value{1.0};
  REQUIRE(v.is<double>());
  REQUIRE(not v.is<bool>());
  REQUIRE(v.as<double>() == 1.0);
}
