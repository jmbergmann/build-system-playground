#pragma once

#include <stdexcept>

namespace errors {

class Error : public std::exception {
public:
  explicit Error(int err) noexcept
  : err_(err) {
  }

  virtual const char* what() const throw();

private:
  const int err_;
};

} // namespace errors
