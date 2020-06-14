#!/bin/sh

# Strip the non-ANSI C comments at the top of the file from
# glslangValidator. $1 is the name of the shader
vfile="$1.vert"
ffile="$1.frag"
v="$1_vert"
f="$1_frag"
vvarname="${v}_src"
fvarname="${f}_src"
vintermediaryfile="$v.h.in"
fintermediaryfile="$f.h.in"
vout="$v.h"
fout="$f.h"
glslangValidator -V --vn $vvarname -o $vintermediaryfile $vfile &
glslangValidator -V --vn $fvarname -o $fintermediaryfile $ffile &
wait
echo "$(tail -n +3 $vintermediaryfile)\n" > $vout \
    && echo "$(tail -n +3 $fintermediaryfile)\n" > $fout \
    && rm $vintermediaryfile $fintermediaryfile

