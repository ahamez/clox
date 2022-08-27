#include <unordered_map>

#include "clox/scanner.hh"

// ---------------------------------------------------------------------------------------------- //

namespace clox {

// ---------------------------------------------------------------------------------------------- //

namespace /* anonymous */
{

[[nodiscard]] bool
is_digit(char c) noexcept
{
  return c >= '0' and c <= '9';
}

[[nodiscard]] bool
is_alpha(char c) noexcept
{
  return (c >= 'a' and c <= 'z') or (c >= 'A' and c <= 'Z') or c == '_';
}

[[nodiscard]] TokenType
identifier_type(const std::string_view& str)
{
  static const auto map =
    std::unordered_map<std::string_view, TokenType>{{"and", TokenType::and_},
                                                    {"class", TokenType::class_},
                                                    {"else", TokenType::else_},
                                                    {"false", TokenType::false_},
                                                    {"false", TokenType::for_},
                                                    {"false", TokenType::fun},
                                                    {"if", TokenType::if_},
                                                    {"nil", TokenType::nil},
                                                    {"or", TokenType::or_},
                                                    {"print", TokenType::print},
                                                    {"return", TokenType::return_},
                                                    {"super", TokenType::super},
                                                    {"this", TokenType::this_},
                                                    {"true", TokenType::true_},
                                                    {"var", TokenType::var},
                                                    {"while", TokenType::while_}};

  if (const auto search = map.find(str); search != cend(map))
  {
    return search->second;
  }
  else
  {
    return TokenType::identifier;
  }
}

} // namespace anonymous

// ---------------------------------------------------------------------------------------------- //

Scanner::Scanner(const std::string& str)
  : token_start_{}
  , current_{cbegin(str)}
  , end_{cend(str)}
  , line_(1)
{}

// ---------------------------------------------------------------------------------------------- //

Token
Scanner::next_token()
{
  skip_whitespaces();

  token_start_ = current_;

  if (current_ == end_)
  {
    return make_token(TokenType::eof);
  }

  const auto c = *current_;
  std::advance(current_, 1);

  if (is_digit(c))
  {
    return make_number_token();
  }
  else if (is_alpha(c))
  {
    return make_identifier_token();
  }

  switch (c)
  {
    case '(':
      return make_token(TokenType::left_paren);
    case ')':
      return make_token(TokenType::right_paren);
    case '{':
      return make_token(TokenType::left_brace);
    case '}':
      return make_token(TokenType::right_brace);
    case ';':
      return make_token(TokenType::semicolon);
    case ',':
      return make_token(TokenType::comma);
    case '.':
      return make_token(TokenType::dot);
    case '-':
      return make_token(TokenType::minus);
    case '+':
      return make_token(TokenType::plus);
    case '*':
      return make_token(TokenType::star);
    case '/':
      return make_token(TokenType::slash);

    case '!':
      return make_token(match_equal() ? TokenType::bang_equal : TokenType::bang);
    case '=':
      return make_token(match_equal() ? TokenType::equal_equal : TokenType::equal);
    case '<':
      return make_token(match_equal() ? TokenType::less_equal : TokenType::less);
    case '>':
      return make_token(match_equal() ? TokenType::greater_equal : TokenType::greater);

    case '"':
      return make_string_token();

    default:
      return make_error_token("Unexpected character");
  }
}

// ---------------------------------------------------------------------------------------------- //

bool
Scanner::match_equal() noexcept
{
  if (current_ == end_)
  {
    return false;
  }

  if (*current_ != '=')
  {
    return false;
  }

  std::advance(current_, 1);

  return true;
}

// ---------------------------------------------------------------------------------------------- //

char
Scanner::peek_next() const noexcept
{
  if (current_ == end_)
  {
    return '\0';
  }
  else
  {
    return *(std::next(current_));
  }
}

// ---------------------------------------------------------------------------------------------- //

void
Scanner::skip_whitespaces() noexcept
{
  while (true)
  {
    switch (*current_)
    {
      case ' ':
      case '\r':
      case '\t':
        std::advance(current_, 1);
        break;

      case '\n':
        line_ += 1;
        std::advance(current_, 1);
        break;

      case '/':
        if (peek_next() == '/')
        {
          // A comment goes until the end of the line
          while (current_ != end_ and *current_ != '\n')
          {
            std::advance(current_, 1);
          }
        }
        else
        {
          return;
        }
        break;

      default:
        return;
    }
  }
}

// ---------------------------------------------------------------------------------------------- //

Token
Scanner::make_token(TokenType type) const noexcept
{
  return Token{type, std::string_view{token_start_, current_}, line_};
}

// ---------------------------------------------------------------------------------------------- //

Token
Scanner::make_token(TokenType type, const std::string_view& token) const noexcept
{
  return Token{type, token, line_};
}

// ---------------------------------------------------------------------------------------------- //

Token
Scanner::make_error_token(const std::string_view& msg) const noexcept
{
  return make_token(TokenType::error, msg);
}

// ---------------------------------------------------------------------------------------------- //

Token
Scanner::make_string_token() noexcept
{
  while (current_ != end_ and *current_ != '"')
  {
    if (*current_ == '\n')
    {
      line_ += 1;
    }
    std::advance(current_, 1);
  }

  if (current_ == end_)
  {
    return make_error_token("Unterminated string");
  }

  // Consume the closing quote.
  std::advance(current_, 1);

  // +1 / -1: remove quotes
  return make_token(TokenType::string, {token_start_ + 1, current_ - 1});
}

// ---------------------------------------------------------------------------------------------- //

Token
Scanner::make_number_token() noexcept
{
  while (is_digit(*current_))
  {
    std::advance(current_, 1);
  }

  // Look for a fractional part.
  if (*current_ == '.' and is_digit(peek_next()))
  {
    // Consume the dot.
    std::advance(current_, 1);

    while (is_digit(*current_))
    {
      std::advance(current_, 1);
    }
  }

  return make_token(TokenType::number);
}

// ---------------------------------------------------------------------------------------------- //

Token
Scanner::make_identifier_token() noexcept
{
  while (is_alpha(*current_) or is_digit(*current_))
  {
    std::advance(current_, 1);
  }

  return make_token(identifier_type({token_start_, current_}));
}

// ---------------------------------------------------------------------------------------------- //

} // clox