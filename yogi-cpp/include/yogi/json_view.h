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

#ifndef YOGI_JSON_VIEW_H
#define YOGI_JSON_VIEW_H

#include "json.h"

#include <string>

namespace yogi {

////////////////////////////////////////////////////////////////////////////////
/// Helper class for passing different types of JSON to functions that
/// internally require a C string holding the serialized JSON data.
///
/// \attention
///   It is imperative that the value passed to any of the view's constructors
///   outlives the view object!
////////////////////////////////////////////////////////////////////////////////
class JsonView {
 public:
  /// Constructs a view that evaluates to a nullptr.
  JsonView() : s_(nullptr) {}

  /// Constructs a view from a standard string.
  ///
  /// \param s Referenced string.
  JsonView(const std::string& s) : s_(s.c_str()) {}

  /// Constructs a view from a NULL-terminated string.
  ///
  /// \param s Referenced NULL-terminated string.
  JsonView(const char* s) : s_(s){};

  /// Constructs a view from a JSON object.
  ///
  /// \param json The JSON object to reference.
  JsonView(const Json& json) : tmp_(json.dump()), s_(tmp_.c_str()){};

  /// Returns a NULL-terminated string holding the serialized JSON data.
  ///
  /// \attention
  ///   The returned value is only valid as long as both the view object and the
  ///   parameter passed to any of its constructors are valid.
  ///
  /// \returns NULL-terminated string holding the serialized JSON data.
  operator const char*() const { return s_; }

 private:
  const std::string tmp_;
  const char* const s_;
};

}  // namespace yogi

#endif  // YOGI_JSON_VIEW_H
