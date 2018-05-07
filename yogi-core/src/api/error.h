#pragma once

#include "../config.h"

#include <stdexcept>

namespace api {

class Error : public std::exception {
 public:
  explicit Error(int err) noexcept : err_(err) {}

  explicit operator bool() const { return err_ != YOGI_OK; }
  bool operator!() const { return err_ == YOGI_OK; }

  int error_code() const noexcept { return err_; }

  virtual const char* what() const throw();

 private:
  const int err_;
};

namespace {

const auto kSuccess = Error(YOGI_OK);

}  // anonymous namespace
}  // namespace api
