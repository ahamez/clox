#pragma once

#include <cstddef>
#include <string>
#include <string_view>

#include "clox/token.hh"

namespace clox {

// ---------------------------------------------------------------------------------------------- //

class Scanner
{
public:
  explicit Scanner(const std::string&);
  Token next_token();

private:
  [[nodiscard]] Token make_token(TokenType) const noexcept;
  [[nodiscard]] Token make_token(TokenType, const std::string_view&) const noexcept;
  [[nodiscard]] Token make_error_token(const std::string_view&) const noexcept;
  [[nodiscard]] Token make_string_token() noexcept;
  [[nodiscard]] Token make_number_token() noexcept;
  [[nodiscard]] Token make_identifier_token() noexcept;
  [[nodiscard]] bool match_equal() noexcept;
  [[nodiscard]] char peek_next() const noexcept;
  void skip_whitespaces() noexcept;

private:
  std::string::const_iterator token_start_{};
  std::string::const_iterator current_{};
  const std::string::const_iterator end_{};
  std::size_t line_{1};
};

// ---------------------------------------------------------------------------------------------- //

} // namespace clox
