VERSION=3.12.1; GIT=https://aomedia.googlesource.com/aom; REF=v3.12.1
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DENABLE_TESTS=OFF -DENABLE_EXAMPLES=OFF
                   -DENABLE_TOOLS=OFF -DENABLE_DOCS=OFF -DCONFIG_PIC=1
                   -DENABLE_NASM=ON -DCONFIG_RUNTIME_CPU_DETECT=0 -DAOM_TARGET_CPU=$( [ "$QNX_ARCH" = aarch64le ] && echo arm64 || echo x86_64 ) )
# Without runtime CPU detection (aom has none for QNX) every Arm extension that
# is compiled in is called unconditionally; keep to ARMv8.0 + NEON, which every
# aarch64 part has. With them, encoding died with SIGILL on a Cortex-A57.
[ "$QNX_ARCH" = aarch64le ] && CMAKE_ARGS_EXTRA+=( -DENABLE_NEON_DOTPROD=0 -DENABLE_NEON_I8MM=0 -DENABLE_SVE=0 -DENABLE_SVE2=0 )
