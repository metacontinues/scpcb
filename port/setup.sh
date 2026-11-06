#!/bin/sh
# configure port/build w/ clang (+ mold, via -Dlinker). extra args go to setup (eg -Dangle=...).
# CC and MUON can be overridden from the env. AR stays gnu ar: muon 0.8.0-dev misdetects llvm-ar
cd "$(dirname "$0")" || exit 1
CC=${CC:-clang} exec ${MUON:-muon} setup "$@" build
