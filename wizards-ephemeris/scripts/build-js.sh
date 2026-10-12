#!/bin/sh
# Build the wasm glue, test it, and pack @magowie/wizards-ephemeris.
# Requires Emscripten 6.0.11 on PATH and Node 20 or newer.
set -eu

root=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
cd "$root"

if ! command -v emcc >/dev/null 2>&1; then
  echo "emcc 6.0.11 is not on PATH. Install emsdk, activate 6.0.11, and source emsdk_env.sh." >&2
  exit 1
fi

emcc_line=$(emcc --version | head -n 1)
if ! printf '%s\n' "$emcc_line" | grep -q '6\.0\.11'; then
  echo "emcc 6.0.11 is required. Found: $emcc_line" >&2
  exit 1
fi

node_major=$(node -p "process.versions.node.split('.')[0]")
if [ "$node_major" -lt 20 ]; then
  echo "Node 20 or newer is required. Found $(node --version)." >&2
  exit 1
fi

emcmake cmake -S . -B build-js -DEPH_JS=ON -DEPH_MINOR_SERIES="${EPH_MINOR_SERIES:-year}"
cmake --build build-js --target eph_js

cd js
npm ci
npm test
npm run build
mkdir -p pack
npm pack --pack-destination pack
