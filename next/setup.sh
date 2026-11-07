#!/bin/sh
# configure next/build w/ the llvm toolchain (clang, llvm-ar; lld via -Dlinker). extra args go to setup.
# CC, AR, MESON (eg MESON=./muon) and BUILD (the dir, default build) can be overridden from the env.
# a static muon's libpkgconf has no default search path, so it gets the system pkg-config's
cd "$(dirname "$0")" || exit 1
if [ -z "$PKG_CONFIG_PATH" ] && command -v pkg-config >/dev/null; then
	PKG_CONFIG_PATH=$(pkg-config --variable pc_path pkg-config) && export PKG_CONFIG_PATH
fi
# muon takes llvm-ar for solaris ar and drops its $out $in, so muon gets gnu ar (indexes the lto bitcode via LLVMgold)
case "${MESON:-meson}" in *muon*) AR=${AR:-ar} ;; esac
CC=${CC:-clang} AR=${AR:-llvm-ar} exec ${MESON:-meson} setup "$@" "${BUILD:-build}"
