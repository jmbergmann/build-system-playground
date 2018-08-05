cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x86-windows-static -DCMAKE_BUILD_TYPE=Debug -G "NMake Makefiles"

nmake yogi-core-test && yogi-core\yogi-core-test.exe --gtest_repeat=1 --gtest_break_on_failure --gtest_catch_exceptions=0 --gtest_filter=*

cmake .. -DCMAKE_BUILD_TYPE=Debug  -DCMAKE_TOOLCHAIN_FILE=~/vcpkg/scripts/buildsystems/vcpkg.cmake

cd "/c/Documents and Settings/commu_000/Documents/Projects/build-system-playground/yogi-python"
YOGI_CORE_LIBRARY=../build/yogi-core/yogi-core-x86.dll python -m unittest

cd ../yogi-dotnet
YOGI_CORE_LIBRARY="/c/Documents and Settings/commu_000/Documents/Projects/build-system-playground/build-x64/yogi-core/yogi-core-x64.dll" dotnet test



TODO:
--------------------------------------------------
* Add FormatDuration method
* Add API function to implement ToString() for objects instead of doing it in each language individually
