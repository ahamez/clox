#pragma once

#include <string_view>

namespace clox {

// ---------------------------------------------------------------------------------------------- //

enum class TokenType
{
  // Single-character tokens
  left_paren,
  right_paren,
  left_brace,
  right_brace,
  comma,
  dot,
  minus,
  plus,
  semicolon,
  slash,
  star,
  // One or two characters tokens
  bang,
  bang_equal,
  equal,
  equal_equal,
  greater,
  greater_equal,
  less,
  less_equal,
  // Literals
  identifier,
  string,
  number,
  //  Keywords
  and_,
  class_,
  else_,
  false_,
  for_,
  fun,
  if_,
  nil,
  or_,
  print,
  return_,
  super,
  this_,
  true_,
  var,
  while_,
  //  Meta
  error,
  eof
};

// ---------------------------------------------------------------------------------------------- //

struct Token
{
  Token()
    : type{TokenType::error}
    , token{}
    , line{}
  {}

  Token(TokenType type, std::string_view token, std::size_t line)
    : type{type}
    , token{token}
    , line{line}
  {}

  TokenType type;
  std::string_view token;
  std::size_t line;
};

// ---------------------------------------------------------------------------------------------- //

} // clox
