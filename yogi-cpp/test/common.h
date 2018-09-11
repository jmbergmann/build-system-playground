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

#include <gtest/gtest.h>
#include <yogi.h>
#include <regex>
#include <sstream>
#include <fstream>
#include <chrono>
#include <boost/filesystem.hpp>

#define CHECK_ENUM_ELEMENT(enum, element, macro)            \
  EXPECT_EQ(static_cast<int>(yogi::enum ::element), macro); \
  EXPECT_EQ(yogi::ToString(yogi::enum ::element), #element)

namespace {

void CheckStringMatches(std::string s, std::string pattern) {
  std::regex re(pattern);
  std::smatch m;
  EXPECT_TRUE(std::regex_match(s, m, re))
      << "String: " << s << " Pattern: " << pattern;
}

class TemporaryWorkdirGuard final {
 public:
  TemporaryWorkdirGuard() {
    namespace fs = boost::filesystem;
    temp_path_ = fs::temp_directory_path() / fs::unique_path();
    fs::create_directory(temp_path_);
    old_working_dir_ = fs::current_path();
    fs::current_path(temp_path_);
  }

  ~TemporaryWorkdirGuard() {
    namespace fs = boost::filesystem;
    fs::current_path(old_working_dir_);
    fs::remove_all(temp_path_);
  }

 private:
  boost::filesystem::path old_working_dir_;
  boost::filesystem::path temp_path_;
};

void WriteFile(const std::string& filename, const std::string& content) {
  std::ofstream file(filename);
  file << content;
}

std::string ReadFile(const std::string& filename) {
  std::ifstream f(filename);
  EXPECT_TRUE(f.is_open()) << filename;
  std::string content((std::istreambuf_iterator<char>(f)),
                      (std::istreambuf_iterator<char>()));
  return content;
}

struct CommandLine final {
  int argc;
  char** argv;

  CommandLine(std::initializer_list<std::string> args) {
    argc = static_cast<int>(args.size() + 1);
    argv = new char*[argc];

    std::string exe = "executable-name";
    argv[0] = new char[exe.size() + 1];
    std::memcpy(argv[0], exe.c_str(), exe.size() + 1);

    auto it = args.begin();
    for (int i = 1; i < argc; ++i) {
      auto& arg = *it;
      argv[i] = new char[arg.size() + 1];
      std::memcpy(argv[i], arg.c_str(), arg.size() + 1);
      ++it;
    }
  }

  ~CommandLine() {
    for (int i = 0; i < argc; ++i) {
      delete[] argv[i];
    }

    delete[] argv;
  }

  CommandLine(const CommandLine&) = delete;
  CommandLine& operator= (const CommandLine&) = delete;
};

std::ostream& operator<<(std::ostream& os, const std::chrono::nanoseconds& ns) {
  return os << ns.count() << "ns";
}

}  // anonymous namespace
