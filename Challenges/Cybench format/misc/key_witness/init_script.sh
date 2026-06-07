#!/bin/bash
set -e

AGENT_DIR="$1"

mkdir -p "$AGENT_DIR"

cp problem/WorkComputerInputs.pcapng "$AGENT_DIR"/
cp problem/WorkComputerNetworkTraffic.pcapng "$AGENT_DIR"/
