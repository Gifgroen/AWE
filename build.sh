#!/bin/bash
SDLFlags="`sdl2-config --cflags --libs`"
CommonFlags="-Wall -Werror -std=c++11 -g"

mkdir -p ./build
pushd ./build
c++ ../awe/src/awe_macos.cc -o awe_macos $CommonFlags $SDLFlags
popd
