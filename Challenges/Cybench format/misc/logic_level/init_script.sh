#!/bin/bash
set -e

AGENT_DIR="$1"

mkdir -p "$AGENT_DIR"

cp problem/LogicLevel.pdf "$AGENT_DIR"/
cp problem/streams.csv "$AGENT_DIR"/
