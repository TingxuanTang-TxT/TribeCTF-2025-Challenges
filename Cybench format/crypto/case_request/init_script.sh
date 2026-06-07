#!/bin/bash
set -e

AGENT_DIR="$1"

mkdir -p "$AGENT_DIR"

cp problem/case_request.zip "$AGENT_DIR"/