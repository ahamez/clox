#pragma once

#include <compare>
#include <iosfwd>
#include <string>
#include <typeindex>

// Avoid an error with clang 15 where std::unary_function has been removed.
// A fix in boost has been commited, but it's not yet available in a release.
// https://stackoverflow.com/a/73364020/21584
#define BOOST_NO_CXX98_FUNCTION_BASE
#include <boost/intrusive/unordered_set.hpp>

namespace clox {

// ---------------------------------------------------------------------------------------------- //

namespace bi = boost::intrusive;

struct ObjString : public bi::unordered_set_base_hook<bi::link_mode<bi::normal_link>>
{
  ObjString(const ObjString&) = delete;
  ~ObjString() = default;
  ObjString(ObjString&&) = delete;
  ObjString& operator=(const ObjString&) = delete;
  ObjString& operator=(ObjString&&) noexcept = delete;

  std::string str;

  explicit ObjString(std::string);

  friend bool operator==(const ObjString&, const ObjString&) noexcept;
  friend std::strong_ordering operator<=>(const ObjString&, const ObjString&) noexcept;

  friend std::ostream& operator<<(std::ostream& os, const ObjString& string);
};

// ---------------------------------------------------------------------------------------------- //

using ObjStringSet = bi::unordered_set<ObjString,
                                       bi::constant_time_size<false>,
                                       bi::power_2_buckets<true>,
                                       bi::hash<std::hash<ObjString>>>;

// ---------------------------------------------------------------------------------------------- //

} // namespace clox

// ---------------------------------------------------------------------------------------------- //

template<>
struct std::hash<clox::ObjString>
{
  std::size_t operator()(const clox::ObjString&) const noexcept;
};

// ---------------------------------------------------------------------------------------------- //
