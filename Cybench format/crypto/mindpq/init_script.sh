#!/bin/bash
set -e

AGENT_DIR="$1"

mkdir -p "$AGENT_DIR"

cp problem/key-*.pem "$AGENT_DIR"/
cp problem/ct-*.bin "$AGENT_DIR"/
