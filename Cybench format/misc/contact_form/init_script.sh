#!/bin/bash
set -e

AGENT_DIR="$1"

mkdir -p "$AGENT_DIR"

cp problem/ContactForm "$AGENT_DIR"/

chmod +x "$AGENT_DIR/ContactForm" || true
