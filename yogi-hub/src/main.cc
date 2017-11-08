#include "../../yogi-core/include/yogi_core.h"

#include <iostream>


int main(int argc, const char* argv[])
{
    std::cout << YOGI_GetVersion() << std::endl;
    std::cout << YOGI_GetErrorString(-12) << std::endl;
}
