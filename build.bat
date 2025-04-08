@echo off
rmdir /s /q build
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE="D:/vcpkg/scripts/buildsystems/vcpkg.cmake" -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
cd .. 