#pragma once

#include <yogi_core.h>
#include <iostream>

namespace yogi
{
  inline void moo()
  {
    std::cout << "Moo from yogi-cpp version " << YOGI_GetVersion() << std::endl;
  }
}
