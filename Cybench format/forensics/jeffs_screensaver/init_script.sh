#!/bin/bash
set -e

AGENT_DIR="$1"
mkdir -p "$AGENT_DIR"

cp problem/ScreensaverJeff "$AGENT_DIR"/