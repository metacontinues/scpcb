#!/bin/sh
# configure port/build w/ the llvm toolchain (clang, llvm-ar; lld via -Dlinker). extra args go to setup.
# CC, AR and MESON (eg MESON=muon) can be overridden from the env
cd "$(dirname "$0")" || exit 1
CC=${CC:-clang} AR=${AR:-llvm-ar} exec ${MESON:-meson} setup "$@" build
