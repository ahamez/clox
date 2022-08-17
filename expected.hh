#pragma once

#include <optional>
#include <stdexcept>
#include <string>

// ---------------------------------------------------------------------------------------------- //

namespace clox {

template<typename T, typename E>
class Expected
{
private:
  Expected()
    : valid_{false}
  {
  }

  union
  {
    T value_;
    E error_;
  };

  bool valid_;

public:
  template<typename... Args>
  static Expected ok(Args&&... args)
  {
    // auto result = Expected{};
    Expected result;
    result.valid_ = true;
    new (&result.value_) T{std::forward<Args>(args)...};

    return result;
  }

  template<typename... Args>
  static Expected error(Args&&... args)
  {
    // auto result = Expected{};
    Expected result;
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

  T& get()
  {
    if (not valid_)
    {
      throw std::logic_error("Missing a valid value");
    }

    return value_;
  }

  const T& get() const
  {
    if (not valid_)
    {
      throw std::logic_error("Missing a valid value");
    }

    return value_;
  }

  E& error()
  {
    if (valid_)
    {
      throw std::logic_error("There is no error");
    }

    return error_;
  }

  const E& error() const
  {
    if (valid_)
    {
      throw std::logic_error("There is no error");
    }

    return error_;
  }
};

// ---------------------------------------------------------------------------------------------- //

} // clox

// ok()

// Expected<int, std::string>
// foo(int x)
//{
//   if (x <= 42) {
//     return Expected<int, std::string>::ok(42);
//   } else {
//     return Expected<int, std::string>::error("larger than universe");
//   }
// }
//
// template<typename Expected>
// void
// print(const Expected& e)
//{
//   if (e) {
//     std::cout << "OK: " << e.get() << '\n';
//   } else {
//     std::cout << "ERROR: " << e.error() << '\n';
//   }
// }
//
// int
// main()
//{
//   const auto res0 = foo(33);
//   print(res0);
//
//   const auto res1 = foo(43);
//   print(res1);
//
//
//   return 0;
// }
