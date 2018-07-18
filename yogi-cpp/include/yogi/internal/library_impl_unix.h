#pragma once

#include <dlfcn.h>

namespace yogi {
namespace internal {

const char* Library::GetFilename() {
  const char* s = std::getenv("YOGI_CORE_LIBRARY");
  return s ? s : "yogi-core.so";
}

Library::LibraryHandle Library::LoadLibrary(const char* filename) {
  dlerror();  // Clear any previous errors
  return dlopen(filename, RTLD_NOW);
}

Library::ProcAddress Library::GetProcAddress(LibraryHandle handle,
                                             const char* name) {
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

}  // namespace internal
}  // namespace yogi
