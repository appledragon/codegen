#!/bin/bash

if test -f "/usr/local/bin/cmake"; then
    cmake=/usr/local/bin/cmake
fi

if test -f "/Applications/CMake.app/Contents/bin/cmake"; then
    cmake=/Applications/CMake.app/Contents/bin/cmake
fi

if ! command -v ninja &> /dev/null
then
    echo "ninja could not be found,use xcode "
    generator=Xcode
else
    echo "ninja found, use ninja "
    generator=Ninja
fi
script_path=$(dirname "$0")
src_path=$script_path
rm -rf "${src_path}"/output/mac/Debug
rm -rf "${src_path}"/build
$cmake -G ${generator} -DCMAKE_OSX_ARCHITECTURES="x86_64" -DCMAKE_BUILD_TYPE=Debug  -Djinja2cpp_DIR=..\Jinja2Cpp\output\lib\jinja2cpp -B "${src_path}"/build "${src_path}"
$cmake --build "${src_path}"/build --config Debug --target install -j 12

