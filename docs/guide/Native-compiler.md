# Compile on the target (native compiler)

`cc`, `c++`, `gcc` and `g++` on the QNX target itself: the ported clang 22 (package `llvm`) with GNU
ld (`binutils`), `make` and `pkg-config`, set up for QNX programs by the package
**`native-toolchain`**. As on QNX Everywhere's images, `cc` is clang and `gcc`/`g++` call clang.
C++ is libc++ on the SDP's `libc++.so.2`, the ABI of every C++ package here, so what you build on
the target links with them.

One part cannot come from a package: QNX's own headers (`<stdio.h>`, `<sys/neutrino.h>`, libc++'s
`<vector>`, ...), the C start files and the link libraries (`libc.so`, `libm.so`, `libc++.so`, ...).
They are QNX's files under QNX's licence, so you copy them from **your own SDP** to the target
(step 2). Do not pass that copy on.

Before this page: a booted target with apk set up ([Install with apk](Install-with-apk)).

## 1. Install the packages

```sh
qnx# apk add native-toolchain        # brings llvm, binutils, make, pkgconf (about 700 MB installed)
qnx# apk add cmake ninja             # optional
```

## 2. Copy QNX's headers and libraries from your SDP

From a clone of the repository on the host (the script uses SSH to the target; about 190 MB on the
target, a few minutes on an emulated aarch64 target):

```sh
host$ git clone --depth 1 https://github.com/openqnx/oss && cd oss
host$ export QNX_SDP_ROOT=$SDP_DIR
host$ scripts/target-sysroot $TARGET_IP
```

It fills `/data/oss/usr/local/qnx-sysroot` (`/usr/local/qnx-sysroot` on the target):

| On the target | From the SDP |
|---|---|
| `qnx-sysroot/usr/include` | `target/qnx/usr/include` (without libstdc++'s headers) |
| `qnx-sysroot/usr/lib/crt1.o crti.o crtn.o` | `target/qnx/<SDP_ARCH>/lib` |
| `qnx-sysroot/usr/lib/crtbegin.o crtend.o libgcc.a libgcc_eh.a` | `host/linux/x86_64/usr/lib/gcc/<triple>/12.2.0` |
| `qnx-sysroot/usr/lib/lib*.so lib*.a` | the QNX OS libraries of `target/qnx/<SDP_ARCH>/lib` and `usr/lib` |

By default only QNX's OS libraries are copied (libc, libm, libc++, libsocket, libscreen, libslog2,
...). `scripts/target-sysroot --all-libs $TARGET_IP` copies every library of the SDP (about 700 MB).
Run the script again after an SDP update.

## 3. Compile

```sh
qnx# . /data/oss/etc/oss-env.sh
qnx# cat > hello.c <<'X'
#include <stdio.h>
#include <process.h>
int main(void) { printf("hello from QNX, thread %d\n", gettid()); return 0; }
X
qnx# cc -O2 -o hello hello.c && ./hello
hello from QNX, thread 1
qnx# c++ -std=c++20 -o app app.cpp
qnx# c++ -o usefmt usefmt.cpp $(pkg-config --cflags --libs fmt)     # after: apk add fmt-dev
qnx# cmake -G Ninja -B build . && ninja -C build
```

## How it is set up

| | |
|---|---|
| `cc`, `gcc` | `exec clang -Qunused-arguments "$@"` (`c++`, `g++`: `clang++`) |
| Configuration | `/usr/local/bin/clang.cfg`, `clang++.cfg`; `clang --version` names the file it read |
| Headers, in order | `/usr/local/include` (the packages), libc++, clang's own, `qnx-sysroot/usr/include` |
| Libraries, in order | `/usr/local/lib` (the packages), `qnx-sysroot/usr/lib` |
| Linker | GNU ld (`binutils`); `-fuse-ld=lld` for LLVM's |
| C++ library | libc++ (`libc++.so.2` of the SDP); libstdc++ is not set up |
| Target | `x86_64-pc-qnx` or `aarch64-unknown-qnx` (clang defines `__QNX__`, `__QNXNTO__`) |

The packages' directories come before QNX's because the SDP also carries older copies of some
libraries that are packages here (zlib, OpenSSL, ...): header and library must be the same one.
Development files of a package are in `<name>-dev` (`apk add fmt-dev`).

## When something does not work

| Message | Cause |
|---|---|
| `cc: no QNX headers in .../qnx-sysroot` or `'stdio.h' file not found` | step 2 was not done for this target |
| `ld: cannot find -lfoo` | a QNX library outside the default set: `scripts/target-sysroot --all-libs`; or a package: `apk add foo-dev` |
| `ld: cannot find crt1.o` | step 2 was not done, or `clang` was called with its own `--sysroot` |
| `cmake` does not find a compiler | `. /data/oss/etc/oss-env.sh` first (`/usr/local/bin` on `PATH`) |
