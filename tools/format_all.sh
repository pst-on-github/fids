#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"

if ! command -v clang-format >/dev/null 2>&1; then
  echo "Error: clang-format not found. Please install it." >&2
  exit 1
fi

# Use project .clang-format for consistent style
find "$ROOT_DIR" -type f \( -name '*.c' -o -name '*.h' \) -print0 | \
  xargs -0 -r clang-format -i

echo "Formatted all .c and .h files with 4-space indent and preserved tabular definitions."