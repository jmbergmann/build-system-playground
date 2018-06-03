#pragma once

#include "../config.h"

#include <stdexcept>
#include <sstream>

namespace api {

class Error : public std::exception {
 public:
  explicit Error(int err) noexcept : err_(err) {}

  explicit operator bool() const { return err_ != YOGI_OK; }
  bool operator!() const { return err_ == YOGI_OK; }
  bool operator==(const Error& rhs) const { return err_ == rhs.err_; }
  bool operator!=(const Error& rhs) const { return err_ != rhs.err_; }

  int error_code() const noexcept { return err_; }

  virtual const char* what() const noexcept;

 private:
  int err_;
};

namespace {

const auto kSuccess = Error(YOGI_OK);

}  // anonymous namespace
}  // namespace api

std::ostream& operator<< (std::ostream& os, const api::Error& err);
