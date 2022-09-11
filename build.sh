#!/bin/bash
mkdir -p ./build
pushd ./build
c++ ../awe/src/awe_macos.cc -o awe_macos -g `sdl2-config --cflags --libs`
popd
