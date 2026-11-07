#!/bin/sh
# lint.sh [build dir]: clang-tidy over our sources w/ the build's compile_commands.json. vendored code is skipped
cd "$(dirname "$0")/.." || exit 1
b=${1:-build}
[ -f "$b/compile_commands.json" ] || { echo "lint.sh: no $b/compile_commands.json, configure first" >&2; exit 1; }
find src tools -name '*.c' ! -path src/plat/glfw.c | sort | xargs run-clang-tidy -quiet -p "$b" 2>/dev/null
