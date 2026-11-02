#!/bin/bash
set -e

cd "$(dirname "$0")"

echo "=== Updating project from Git ==="
git pull --ff-only origin main

echo "=== Configuring CMake ==="
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug

echo "=== Building project ==="
cmake --build build

echo "=== Running Hello World ==="
./build/hello

echo "=== Done ==="
