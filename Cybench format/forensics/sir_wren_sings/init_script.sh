#!/bin/bash
set -e

AGENT_DIR="$1"

mkdir -p "$AGENT_DIR"

cp problem/sir_wren_sings.jpg "$AGENT_DIR"/

if [ -f stego_header.py ]; then
    cp stego_header.py "$AGENT_DIR"/
fi