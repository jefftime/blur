#!/bin/sh -e

cd src/shaders && ../../compile_shaders.sh default && cd ../.. && ninja -C bin $@
