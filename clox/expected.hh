#pragma once

#include <optional>
#include <stdexcept>
#include <string>

// ---------------------------------------------------------------------------------------------- //

// NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)

namespace clox {

template<typename T, typename E>
class Expected
{
private:
  Expected()
    : valid_{false}
  {}

  union
  {
    T value_;
    E error_;
  };

  bool valid_; // NOLINT(modernize-use-default-member-init)

public:
  template<typename... Args>
  static Expected ok(Args&&... args)
  {
    auto result = Expected{};
    result.valid_ = true;
    new (&result.value_) T{std::forward<Args>(args)...};

    return result;
  }

  template<typename... Args>
  static Expected error(Args&&... args)
  {
    auto result = Expected{};
    result.valid_ = false;
    new (&result.error_) E{std::forward<Args>(args)...};

    return result;
  }

  ~Expected()
  {
    if (valid_)
    {
      value_.~T();
    }
    else
    {
      error_.~E();
    }
  }

  Expected(const Expected& rhs)
    : valid_{rhs.valid_}
  {
    if (valid_)
    {
      new (&value_) T{rhs.value_};
    }
    else
    {
      new (&error_) E{rhs.error_};
    }
  }
  Expected& operator=(const Expected&) = default;

  Expected(Expected&& rhs) noexcept
    : valid_{rhs.valid_}
  {
    if (valid_)
    {
      new (&value_) T{std::move(rhs.value_)};
    }
    else
    {
      new (&error_) E{std::move(rhs.error_)};
    }
  }
  Expected& operator=(Expected&&) noexcept = default;

  void swap(Expected& rhs)
  {
    using std::swap;

    if (valid_)
    {
      if (rhs.valid_)
      {
        swap(value_, rhs.value_);
      }
      else
      {
        auto tmp_rhs_error = std::move(rhs.error_);
        rhs.error_.~E();

        new (&rhs.value_) T{std::move(value_)};
        value_.~T();

        new (&error_) E{std::move(tmp_rhs_error)};

        std::swap(valid_, rhs.valid_);
      }
    }
    else
    {
      if (rhs.valid_)
      {
        rhs.swap(*this);
      }
      else
      {
        swap(error_, rhs.error_);
      }
    }
  }

  Expected& operator=(Expected other)
  {
    swap(other);
    return *this;
  }

  explicit operator bool() const { return valid_; }

  explicit operator std::optional<T>() const
  {
    if (valid_)
    {
      return value_;
    }
    else
    {
      return std::optional<T>{};
    }
  }

  [[nodiscard]] T& get()
  {
    if (not valid_)
    {
      throw std::logic_error("Missing a valid value");
    }

    return value_;
  }

  [[nodiscard]] const T& get() const
  {
    if (not valid_)
    {
      throw std::logic_error("Missing a valid value");
    }

    return value_;
  }

  [[nodiscard]] E& error()
  {
    if (valid_)
    {
      throw std::logic_error("There is no error");
    }

    return error_;
  }

  [[nodiscard]] const E& error() const
  {
    if (valid_)
    {
      throw std::logic_error("There is no error");
    }

    return error_;
  }
};

// ---------------------------------------------------------------------------------------------- //

} // namespace clox

// NOLINTEND(cppcoreguidelines-pro-type-union-access)
