#pragma once

#include <iosfwd>
#include <string>

#include <boost/intrusive/slist.hpp>

namespace clox {

// ---------------------------------------------------------------------------------------------- //

namespace bi = boost::intrusive;

struct ObjString
{
  std::string str;

  bi::slist_member_hook<bi::link_mode<bi::auto_unlink>> member_hook_;

  explicit ObjString(std::string str)
    : str{std::move(str)}
  {}

  bool operator==(const ObjString& rhs) const { return str == rhs.str; }
  bool operator!=(const ObjString& rhs) const { return !(rhs == *this); }
  friend std::ostream& operator<<(std::ostream& os, const ObjString& string)
  {
    return os << string.str;
  }
};

// ---------------------------------------------------------------------------------------------- //

namespace detail {

using MemberHookOption = bi::member_hook<ObjString,
                                         bi::slist_member_hook<bi::link_mode<bi::auto_unlink>>,
                                         &ObjString::member_hook_>;

}

using ObjStringList = bi::slist<ObjString, bi::constant_time_size<false>, detail::MemberHookOption>;

// ---------------------------------------------------------------------------------------------- //

}
