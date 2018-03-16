#pragma once

#include "config.h"

#include <stdexcept>

namespace errors {

class Error : public std::exception {
public:
  explicit Error(int err) noexcept
  : err_(err) {
  }

  const int error_code() const noexcept {
    return err_;
  }

  virtual const char* what() const throw();

private:
  const int err_;
};

} // namespace errors
