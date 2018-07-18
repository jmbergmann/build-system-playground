#pragma once

#include <yogi_core.h>

#include <cstdlib>
#include <string>
#include <iostream>

#ifdef _WIN32
# include <Windows.h>
#else
# include <dlfcn.h>
#endif

namespace yogi {
namespace internal {

class Library final {
 public:
  Library() {
    if (!lib_handle_) {
      lib_handle_ = LoadYogiCore();
    }
  }

  ~Library() {
    // Unload DLL
  }

  template <typename Fn>
  static Fn GetFunctionAddress(const char* name) {
    using namespace std::string_literals;

    auto addr = GetProcAddress(lib_handle_, name);
    if (!addr) {
      DieWithErrorMessage("Could not find function "s + name + " in the loaded yogi-core library");
    }

    return reinterpret_cast<Fn>(addr);
  }

#ifdef _WIN32
  typedef HMODULE LibraryHandle;
  typedef FARPROC ProcAddress;
#else
  typedef void* LibraryHandle;
  typedef void* ProcAddress;
#endif

  static LibraryHandle lib_handle_;

  static LibraryHandle LoadYogiCore() {
    using namespace std::string_literals;

    auto filename = GetFilename();
    auto handle = LoadLibrary(filename);
    if (!handle) {
      DieWithErrorMessage("Could not load "s + filename);
    }

    return handle;
  }

  static void DieWithErrorMessage(std::string prefix) {
    std::cerr << "FATAL: " << prefix << ": " << GetLastErrorString() << std::endl;
    std::exit(1);
  }

  // Methods below are platform-dependent
  static const char* GetFilename();
  static LibraryHandle LoadLibrary(const char* filename);
  static ProcAddress GetProcAddress(LibraryHandle handle, const char* name);
  static std::string GetLastErrorString();
};

#ifdef _WIN32

const char* Library::GetFilename() {
  #pragma warning(push)
  #pragma warning(disable : 4996)  // 'getenv': This function or variable may be unsafe.
  const char* s = std::getenv("YOGI_CORE_LIBRARY");
  #pragma warning(pop)
  return s ? s : "yogi-core.so";
}

Library::LibraryHandle Library::LoadLibrary(const char* filename) {
  return ::LoadLibrary(filename);
}

Library::ProcAddress Library::GetProcAddress(LibraryHandle handle, const char* name) {
  return ::GetProcAddress(handle, name);
}

std::string Library::GetLastErrorString() {
  std::string s;
  auto err = GetLastError();
  if (err) {
    LPSTR msg = nullptr;
    auto n = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER
                           | FORMAT_MESSAGE_FROM_SYSTEM
                           | FORMAT_MESSAGE_IGNORE_INSERTS,
                           nullptr,
                           err,
                           MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                           reinterpret_cast<LPTSTR>(&msg),
                           0, nullptr);
    if (n) {
      s.assign(msg, msg + n);
      LocalFree(msg);
    }
  }

  return s;
}

Library::LibraryHandle __declspec(selectany) Library::lib_handle_;

Library __declspec(selectany) library;

#else

const char* Library::GetFilename() {
  const char* s = std::getenv("YOGI_CORE_LIBRARY");
  return s ? s : "yogi-core.so";
}

Library::LibraryHandle Library::LoadLibrary(const char* filename) {
  dlerror();  // Clear any previous errors
  return dlopen(filename, RTLD_NOW);
}

Library::ProcAddress Library::GetProcAddress(LibraryHandle handle, const char* name) {
  dlerror();  // Clear any previous errors
  return dlsym(handle, name);
}

std::string Library::GetLastErrorString() {
  std::string s;
  auto err = dlerror();
  if (err) {
    s.assign(err);
  }

  return s;
}

Library::LibraryHandle __attribute__((weak)) Library::lib_handle_;

Library __attribute__((weak)) library;

#endif

}  // namespace internal
}  // namespace yogi
