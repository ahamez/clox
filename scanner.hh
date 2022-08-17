#pragma once

#include <string>
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
  TokenType type;
  std::string_view token;
  std::size_t line;
};

// ---------------------------------------------------------------------------------------------- //

class Scanner
{
public:
  explicit Scanner(const std::string&);
  Token next_token();

private:
  [[nodiscard]] Token make_token(TokenType) const noexcept;
  [[nodiscard]] Token make_error_token(const std::string_view&) const noexcept;
  [[nodiscard]] Token make_string_token() noexcept;
  [[nodiscard]] Token make_number_token() noexcept;
  [[nodiscard]] Token make_identifier_token() noexcept;
  [[nodiscard]] bool match_equal() noexcept;
  [[nodiscard]] char peek_next() const noexcept;
  void skip_whitespaces() noexcept;

private:
  std::string::const_iterator token_start_;
  std::string::const_iterator current_;
  const std::string::const_iterator end_;
  std::size_t line_;
};

// ---------------------------------------------------------------------------------------------- //

} // clox
