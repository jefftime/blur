#!/bin/sh

# Strip the non-ANSI C comments at the top of the file from
# glslangValidator. $1 is the name of the shader
glslangValidator -V --vn "v$1_src" -o "v$1.h.in" "$1.vert" \
    && glslangValidator -V --vn "f$1_src" -o "f$1.h.in" "$1.frag" \
    && echo "$(tail -n +3 v$1.h.in)\n" > "v$1.h" \
    && echo "$(tail -n +3 f$1.h.in)\n" > "f$1.h"
rm "v$1.h.in" "f$1.h.in"

