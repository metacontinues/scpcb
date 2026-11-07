#!/bin/sh
# configure next/build w/ the llvm toolchain (clang, llvm-ar; lld via -Dlinker). extra args go to setup.
# CC, AR, MESON (eg MESON=./muon), BUILD (the dir, default build or build-$TARGET) and TARGET can be set from the env.
# TARGET=windows cross builds w/ cross/windows.ini (meson only so far). -march=native makes no sense there, so it's off.
# a static muon's libpkgconf has no default search path, so it gets the system pkg-config's
cd "$(dirname "$0")" || exit 1
if [ -z "$PKG_CONFIG_PATH" ] && command -v pkg-config >/dev/null; then
	PKG_CONFIG_PATH=$(pkg-config --variable pc_path pkg-config) && export PKG_CONFIG_PATH
fi
# muon takes llvm-ar for solaris ar and drops its $out $in, so muon gets gnu ar (indexes the lto bitcode via LLVMgold)
case "${MESON:-meson}" in *muon*) AR=${AR:-ar} ;; esac
if [ -n "$TARGET" ]; then
	[ -f "cross/$TARGET.ini" ] || { echo "setup.sh: no cross/$TARGET.ini" >&2; exit 1; }
	# the cross file names the compiler; pkg-config would hand out the host's libs
	unset PKG_CONFIG_PATH
	exec ${MESON:-meson} setup --cross-file "cross/$TARGET.ini" -Dmarch= "$@" "${BUILD:-build-$TARGET}"
fi
CC=${CC:-clang} AR=${AR:-llvm-ar} exec ${MESON:-meson} setup "$@" "${BUILD:-build}"
