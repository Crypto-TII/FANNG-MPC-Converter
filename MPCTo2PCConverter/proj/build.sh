# !/bin/bash
rm -rf build/*
cmake -DCMAKE_PREFIX_PATH=/usr/lib/x86_64-linux-gnu/cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake -S . -B build -G "Unix Makefiles"
cmake --build build --target all -j 22 --