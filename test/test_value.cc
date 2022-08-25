#include <catch2/catch_test_macros.hpp>

#include "clox/value.hh"

using namespace clox;

TEST_CASE("Boolean", "[Value]")
{
  SECTION("Creation")
  {
    const auto v = Value{true};
    REQUIRE(v.is<bool>());
    REQUIRE(not v.is<double>());
    REQUIRE(v.as<bool>() == true);
  }

  SECTION("Equality")
  {
    const auto v0 = Value{true};
    const auto v1 = Value{true};
    const auto v2 = Value{false};
    REQUIRE(v0 == v1);
    REQUIRE(v1 == v0);
    REQUIRE(v0 != v2);
    REQUIRE(v1 != v2);
  }
}

TEST_CASE("Double", "[Value]")
{
  SECTION("Creation")
  {
    const auto v = Value{1.0};
    REQUIRE(v.is<double>());
    REQUIRE(not v.is<bool>());
    REQUIRE(v.as<double>() == 1.0);
  }
  SECTION("Equality")
  {
    const auto v0 = Value{1.0};
    const auto v1 = Value{1.0};
    const auto v2 = Value{1.1};
    REQUIRE(v0 == v1);
    REQUIRE(v1 == v0);
    REQUIRE(v0 != v2);
    REQUIRE(v1 != v2);
  }
}
