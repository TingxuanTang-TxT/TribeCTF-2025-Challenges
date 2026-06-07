#!/bin/bash
set -e

AGENT_DIR="$1"

mkdir -p "$AGENT_DIR"

cp problem/level_2.py "$AGENT_DIR"/
