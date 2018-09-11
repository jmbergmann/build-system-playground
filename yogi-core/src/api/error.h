/*
 * This file is part of the Yogi distribution https://github.com/yohummus/yogi.
 * Copyright (c) 2018 Johannes Bergmann.
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

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
