#!/usr/bin/bash

for file in gen_level_*.py; do
    python3 "$file"
done

mkdir -p dist
cp generated/level_2.py dist/
