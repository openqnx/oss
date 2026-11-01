# fft2d 2006.12.28

> General purpose 1D/2D/3D FFT, DCT and DST package (Ooura)

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://www.kurims.kyoto-u.ac.jp/~ooura/fft2d.tgz |
| SHA-256 | `ada7e99087c4ed477bfdf11413f2ba8db8a840ba9bbf8ac94f4f3972e2a7cec9` |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_fft2d`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `fft2d` (programs, shared libraries, run-time data) and `fft2d-dev` (headers, pkg-config/CMake files, static libraries; needs `fft2d`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`fft2d`](https://openqnx.github.io/oss/package/x86_64/fft2d.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/fft2d.html)) · [`fft2d-dev`](https://openqnx.github.io/oss/package/x86_64/fft2d-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/fft2d-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add fft2d` (dependencies come along); one version: `apk add fft2d=<version>` | `apk del fft2d` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install fft2d` (dependencies first) | `oss-get remove fft2d` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`fft2d-dev` the same way, by its own name (`apk add fft2d-dev`, `oss-get install fft2d-dev`, by hand with its own file and list `fft2d-dev.list`); remove it before `fft2d`, which it needs.

```sh
# on the QNX target, as root
apk add fft2d fft2d-dev
apk add qnx-test-fft2d && /usr/local/bin/qnx-port-tests/test_fft2d    # its on-target test, from tests-8.0-<arch>
apk del fft2d fft2d-dev qnx-test-fft2d
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/fft2d.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/fft2d.list
rm var/lib/manual/fft2d.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild fft2d` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

No other component has to be built first.

### 2. Get the upstream source

```bash
URL=https://www.kurims.kyoto-u.ac.jp/~ooura/fft2d.tgz
mkdir -p dist src/fft2d
curl -fL -o dist/$(basename $URL) $URL
echo "ada7e99087c4ed477bfdf11413f2ba8db8a840ba9bbf8ac94f4f3972e2a7cec9  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/fft2d --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/fft2d; B=build/$QNX_ARCH/fft2d; mkdir -p $B
```

```bash
do_build() {
  local f objs=
  { echo "FFT2D_0 { global:"; for f in $FFT2D_API; do echo "  $f;"; done; echo "local: *; };"; } > "$B/fft2d.map"
  for f in fftsg fftsg2d fftsg3d shrtdct; do
    qcc -V$QCC_TARGET -O2 -fPIC -D_QNX_SOURCE -c "$S/$f.c" -o "$B/$f.o" || return 1
    objs="$objs $B/$f.o"
  done
  qcc -V$QCC_TARGET -shared -Wl,-soname,libfft2d.so.0 -Wl,--version-script,"$B/fft2d.map" \
    -o "$B/libfft2d.so.0" $objs -lm
}
```

```bash
do_install() {
  local L="$STAGE$PREFIX/lib"
  install -d "$L/pkgconfig" "$STAGE$PREFIX/include" "$STAGE$PREFIX/share/doc/fft2d" &&
  install -m755 "$B/libfft2d.so.0" "$L/" && ln -sf libfft2d.so.0 "$L/libfft2d.so" &&
  install -m644 "$S/readme2d.txt" "$STAGE$PREFIX/share/doc/fft2d/" &&
  cat > "$STAGE$PREFIX/include/fft2d.h" <<'H'
/* Ooura's General Purpose FFT package (libfft2d): the entry points of
   fftsg.c, fftsg2d.c, fftsg3d.c and shrtdct.c. The argument conventions
   (sizes powers of 2, isgn, the ip/w work areas with ip[0] = 0 on the first
   call, t work area or NULL) are described at the top of each source file;
   see also share/doc/fft2d/readme2d.txt. Written for this package: upstream
   ships no header. */
#ifndef FFT2D_H
#define FFT2D_H
#ifdef __cplusplus
extern "C" {
#endif
/* 1D (fftsg.c) */
void cdft(int n, int isgn, double *a, int *ip, double *w);
void rdft(int n, int isgn, double *a, int *ip, double *w);
void ddct(int n, int isgn, double *a, int *ip, double *w);
void ddst(int n, int isgn, double *a, int *ip, double *w);
void dfct(int n, double *a, double *t, int *ip, double *w);
void dfst(int n, double *a, double *t, int *ip, double *w);
/* 2D (fftsg2d.c): a[0...n1-1][0...n2-1] */
void cdft2d(int n1, int n2, int isgn, double **a, double *t, int *ip, double *w);
void rdft2d(int n1, int n2, int isgn, double **a, double *t, int *ip, double *w);
void rdft2dsort(int n1, int n2, int isgn, double **a);
void ddct2d(int n1, int n2, int isgn, double **a, double *t, int *ip, double *w);
void ddst2d(int n1, int n2, int isgn, double **a, double *t, int *ip, double *w);
/* 3D (fftsg3d.c): a[0...n1-1][0...n2-1][0...n3-1] */
void cdft3d(int n1, int n2, int n3, int isgn, double ***a, double *t, int *ip, double *w);
void rdft3d(int n1, int n2, int n3, int isgn, double ***a, double *t, int *ip, double *w);
void rdft3dsort(int n1, int n2, int n3, int isgn, double ***a);
void ddct3d(int n1, int n2, int n3, int isgn, double ***a, double *t, int *ip, double *w);
void ddst3d(int n1, int n2, int n3, int isgn, double ***a, double *t, int *ip, double *w);
/* 8x8 and 16x16 DCT (shrtdct.c): a[0...7][0...7], a[0...15][0...15] */
void ddct8x8s(int isgn, double **a);
void ddct16x16s(int isgn, double **a);
#ifdef __cplusplus
}
#endif
#endif
H
  cat > "$L/pkgconfig/fft2d.pc" <<PC
prefix=$PREFIX
libdir=\${prefix}/lib
includedir=\${prefix}/include

Name: fft2d
Description: Ooura's general purpose 1D/2D/3D FFT package
Version: $VERSION
Libs: -L\${libdir} -lfft2d
Cflags: -I\${includedir}
PC
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Takuya Ooura's General Purpose FFT package (fft2d.tgz, 2006-12-28): 1D, 2D and 3D complex/real DFT, DCT and DST (split-radix "fftsg" versions) and the 8x8/16x16 DCTs. Upstream has no version number; this is the date of the only release of the 2D package, the same file TensorFlow pinned.
- The upstream server is often unreachable from outside Japan; the Internet Archive's capture of that same URL is the fallback. Either way the file must match the pinned checksum.
- Upstream ships C files to copy into a project, no library and no header. Built here as libfft2d.so.0 from fftsg.c, fftsg2d.c, fftsg3d.c and shrtdct.c (fft4f2d.c is the older 2D version with the same function names). Only the documented entry points are exported (version script): the internal helpers have generic names (makewt, bitrv2, cftfsub, ...) that would collide with other copies of Ooura's code. fft2d.h declares them, with the argument descriptions in the source files. Single-threaded (no USE_FFT2D_PTHREADS), as TensorFlow builds it.
- ifndef FFT2D_H define FFT2D_H ifdef __cplusplus
- endif
- ifdef __cplusplus
- endif endif

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH fft2d`.

### 5. What it installs

5 files under `/usr/local`: 2 shared-library entries, 1 headers.

```
/usr/local/lib/libfft2d.so
/usr/local/lib/libfft2d.so.0
```

The complete list is `stage/<arch>/.manifest/fft2d` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa fft2d
ls dists/8.0/*/ | grep '^fft2d-'
```

This writes `fft2d-<ver>-r0.tar.gz` (run time) and `fft2d-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add fft2d
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/fft2d-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/fft2d-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_fft2d`](../../tests/src/) links fft2d the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-fft2d`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-fft2d
/usr/local/bin/qnx-port-tests/test_fft2d
```

```
== fft2d ==
  ok   ...
PASS fft2d
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_fft2d` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_fft2d`](../../tests/src/) -- a program linked against fft2d the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - rdft([1 2 3 4]) = 10, -2, -2, -2 (Ooura's layout and sign)
   - cdft of an impulse: all ones
   - rdft forward + inverse x 2/n = input
   - ddct forward + inverse round trip
   - ddst forward + inverse round trip
   - cdft2d 4 x 4 complex forward + inverse / (n1 n2)
   - ddct2d 4 x 8 forward + inverse round trip
   - rdft3d 4x4x4 forward + inverse x 2/(n1 n2 n3)
   - ddct8x8s: DC coefficient = sum / 8 (normalized DCT)
   - ddct8x8s forward + inverse = input (orthonormal)

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | fft2d 2006.12.28-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/fft2d-2006.12.28-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | fft2d 2006.12.28-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/fft2d-2006.12.28-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2006.12.28**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add fft2d=<version>`), sources in `ported/fft2d/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates fft2d`; port one with `scripts/port-update fft2d`.

<details><summary>recipe.sh</summary>

```bash
# Takuya Ooura's General Purpose FFT package (fft2d.tgz, 2006-12-28): 1D,
# 2D and 3D complex/real DFT, DCT and DST (split-radix "fftsg" versions) and
# the 8x8/16x16 DCTs. Upstream has no version number; this is the date of
# the only release of the 2D package, the same file TensorFlow pinned.
VERSION=2006.12.28
TARBALL=https://www.kurims.kyoto-u.ac.jp/~ooura/fft2d.tgz
SHA256=ada7e99087c4ed477bfdf11413f2ba8db8a840ba9bbf8ac94f4f3972e2a7cec9
BUILDSYS=custom
# The upstream server is often unreachable from outside Japan; the Internet
# Archive's capture of that same URL is the fallback. Either way the file
# must match the pinned checksum.
FFT2D_MIRROR="https://web.archive.org/web/20250108162814id_/$TARBALL"
do_fetch() {
  local tb="$WS/dist/fft2d-$VERSION.tgz"; mkdir -p "$WS/dist" "$SRCROOT"
  [ -f "$tb" ] || curl -fsSL -m 60 --retry 2 -o "$tb" "$TARBALL" || curl -fsSL --retry 3 -o "$tb" "$FFT2D_MIRROR" || { rm -f "$tb"; return 1; }
  echo "$SHA256  $tb" | sha256sum -c --status || { echo "checksum mismatch: $tb"; rm -f "$tb"; return 1; }
  tar xzf "$tb" -C "$SRCROOT" --strip-components=1
}
# Upstream ships C files to copy into a project, no library and no header.
# Built here as libfft2d.so.0 from fftsg.c, fftsg2d.c, fftsg3d.c and
# shrtdct.c (fft4f2d.c is the older 2D version with the same function
# names). Only the documented entry points are exported (version script):
# the internal helpers have generic names (makewt, bitrv2, cftfsub, ...)
# that would collide with other copies of Ooura's code. fft2d.h declares
# them, with the argument descriptions in the source files. Single-threaded
# (no USE_FFT2D_PTHREADS), as TensorFlow builds it.
FFT2D_API="cdft rdft ddct ddst dfct dfst cdft2d rdft2d rdft2dsort ddct2d ddst2d
           cdft3d rdft3d rdft3dsort ddct3d ddst3d ddct8x8s ddct16x16s"
do_build() {
  local f objs=
  { echo "FFT2D_0 { global:"; for f in $FFT2D_API; do echo "  $f;"; done; echo "local: *; };"; } > "$B/fft2d.map"
  for f in fftsg fftsg2d fftsg3d shrtdct; do
    qcc -V$QCC_TARGET -O2 -fPIC -D_QNX_SOURCE -c "$S/$f.c" -o "$B/$f.o" || return 1
    objs="$objs $B/$f.o"
  done
  qcc -V$QCC_TARGET -shared -Wl,-soname,libfft2d.so.0 -Wl,--version-script,"$B/fft2d.map" \
    -o "$B/libfft2d.so.0" $objs -lm
}
do_install() {
  local L="$STAGE$PREFIX/lib"
  install -d "$L/pkgconfig" "$STAGE$PREFIX/include" "$STAGE$PREFIX/share/doc/fft2d" &&
  install -m755 "$B/libfft2d.so.0" "$L/" && ln -sf libfft2d.so.0 "$L/libfft2d.so" &&
  install -m644 "$S/readme2d.txt" "$STAGE$PREFIX/share/doc/fft2d/" &&
  cat > "$STAGE$PREFIX/include/fft2d.h" <<'H'
/* Ooura's General Purpose FFT package (libfft2d): the entry points of
   fftsg.c, fftsg2d.c, fftsg3d.c and shrtdct.c. The argument conventions
   (sizes powers of 2, isgn, the ip/w work areas with ip[0] = 0 on the first
   call, t work area or NULL) are described at the top of each source file;
   see also share/doc/fft2d/readme2d.txt. Written for this package: upstream
   ships no header. */
#ifndef FFT2D_H
#define FFT2D_H
#ifdef __cplusplus
extern "C" {
#endif
/* 1D (fftsg.c) */
void cdft(int n, int isgn, double *a, int *ip, double *w);
void rdft(int n, int isgn, double *a, int *ip, double *w);
void ddct(int n, int isgn, double *a, int *ip, double *w);
void ddst(int n, int isgn, double *a, int *ip, double *w);
void dfct(int n, double *a, double *t, int *ip, double *w);
void dfst(int n, double *a, double *t, int *ip, double *w);
/* 2D (fftsg2d.c): a[0...n1-1][0...n2-1] */
void cdft2d(int n1, int n2, int isgn, double **a, double *t, int *ip, double *w);
void rdft2d(int n1, int n2, int isgn, double **a, double *t, int *ip, double *w);
void rdft2dsort(int n1, int n2, int isgn, double **a);
void ddct2d(int n1, int n2, int isgn, double **a, double *t, int *ip, double *w);
void ddst2d(int n1, int n2, int isgn, double **a, double *t, int *ip, double *w);
/* 3D (fftsg3d.c): a[0...n1-1][0...n2-1][0...n3-1] */
void cdft3d(int n1, int n2, int n3, int isgn, double ***a, double *t, int *ip, double *w);
void rdft3d(int n1, int n2, int n3, int isgn, double ***a, double *t, int *ip, double *w);
void rdft3dsort(int n1, int n2, int n3, int isgn, double ***a);
void ddct3d(int n1, int n2, int n3, int isgn, double ***a, double *t, int *ip, double *w);
void ddst3d(int n1, int n2, int n3, int isgn, double ***a, double *t, int *ip, double *w);
/* 8x8 and 16x16 DCT (shrtdct.c): a[0...7][0...7], a[0...15][0...15] */
void ddct8x8s(int isgn, double **a);
void ddct16x16s(int isgn, double **a);
#ifdef __cplusplus
}
#endif
#endif
H
  cat > "$L/pkgconfig/fft2d.pc" <<PC
prefix=$PREFIX
libdir=\${prefix}/lib
includedir=\${prefix}/include

Name: fft2d
Description: Ooura's general purpose 1D/2D/3D FFT package
Version: $VERSION
Libs: -L\${libdir} -lfft2d
Cflags: -I\${includedir}
PC
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
