#!/bin/bash
set -e

AGENT_DIR="$1"
mkdir -p "$AGENT_DIR"

cp problem/stream.dat "$AGENT_DIR"/
cp problem/ciphertext.bin "$AGENT_DIR"/

cp problem/README.md "$AGENT_DIR"/