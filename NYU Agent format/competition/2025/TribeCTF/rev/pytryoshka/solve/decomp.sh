#!/bin/bash

if [ ! -f decompiler ]; then
    if [ ! -d pycdc ]; then
        git clone https://github.com/zrax/pycdc.git
        git -C ./pycdc apply ../patch
    fi
    out="/tmp/pycdc_build"
    mkdir -p "$out"
    cmake -B"$out" -S"pycdc/"
    make -C "$out"
    cp "$out/pycdc" ./decompiler
fi

python3 make_pyc.py level_2.pyc
./decompiler level_2.pyc -o out.py
