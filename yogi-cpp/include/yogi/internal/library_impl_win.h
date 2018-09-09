#ifndef YOGI_INTERNAL_LIBRARY_IMPL_WIN_H
#define YOGI_INTERNAL_LIBRARY_IMPL_WIN_H

#include <Windows.h>

namespace yogi {
namespace internal {

const char* Library::GetFilename() {
#pragma warning(push)
#pragma warning( \
    disable : 4996)  // 'getenv': This function or variable may be unsafe.
  const char* s = std::getenv("YOGI_CORE_LIBRARY");
#pragma warning(pop)

  if (!s) {
    s = sizeof(void*) == 4 ? "yogi-core-x86.dll" : "yogi-core-x64.dll";
  }

  return s;
}

Library::LibraryHandle Library::LoadLibrary(const char* filename) {
  return ::LoadLibraryA(filename);
}

Library::ProcAddress Library::GetProcAddress(LibraryHandle handle,
                                             const char* name) {
  return ::GetProcAddress(handle, name);
}

std::string Library::GetLastErrorString() {
  std::string s;
  auto err = GetLastError();
  if (err) {
    LPSTR msg = nullptr;
    auto n = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPTSTR>(&msg), 0, nullptr);
    if (n) {
      s.assign(msg, msg + n);
      LocalFree(msg);
    }
  }

  return s;
}

}  // namespace internal
}  // namespace yogi

#endif  // YOGI_INTERNAL_LIBRARY_IMPL_WIN_H
