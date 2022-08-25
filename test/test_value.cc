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

TEST_CASE("String", "[Value]")
{
  SECTION("Creation")
  {
    auto obj = std::make_unique<clox::ObjString>("foo");
    const auto v = Value{obj.get()};

    REQUIRE(v.is<const clox::ObjString*>());
    REQUIRE(not v.is<bool>());
    REQUIRE(v.as<const clox::ObjString*>()->str == "foo");
  }
  SECTION("Equality", "[value]")
  {
    auto obj0 = std::make_unique<clox::ObjString>("foo");
    const auto v0 = Value{obj0.get()};
    auto obj1 = std::make_unique<clox::ObjString>("foo");
    const auto v1 = Value{obj1.get()};
    auto obj2 = std::make_unique<clox::ObjString>("bar");
    const auto v2 = Value{obj2.get()};

    REQUIRE(v0 == v1);
    REQUIRE(v1 == v0);
    REQUIRE(v0 != v2);
    REQUIRE(v1 != v2);
  }
}
