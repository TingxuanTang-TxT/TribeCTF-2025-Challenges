#!/bin/bash
set -e

AGENT_DIR="$1"

mkdir -p "$AGENT_DIR"

cp problem/ducker-hunt.tar "$AGENT_DIR"/