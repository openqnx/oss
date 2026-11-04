# fftw 3.3.10

> Discrete Fourier transform (FFT) library, double and single precision

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://www.fftw.org/fftw-3.3.10.tar.gz |
| SHA-256 | `56c932549852cddcfafdab3820b0200c7742675be92179e59e6215b340e26467` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_fftw`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `fftw` (programs, shared libraries, run-time data) and `fftw-dev` (headers, pkg-config/CMake files, static libraries; needs `fftw`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`fftw`](https://openqnx.github.io/oss/package/x86_64/fftw.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/fftw.html)) · [`fftw-dev`](https://openqnx.github.io/oss/package/x86_64/fftw-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/fftw-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add fftw` (dependencies come along); one version: `apk add fftw=<version>` | `apk del fftw` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install fftw` (dependencies first) | `oss-get remove fftw` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`fftw-dev` the same way, by its own name (`apk add fftw-dev`, `oss-get install fftw-dev`, by hand with its own file and list `fftw-dev.list`); remove it before `fftw`, which it needs.

```sh
# on the QNX target, as root
apk add fftw fftw-dev
apk add qnx-test-fftw && /usr/local/bin/qnx-port-tests/test_fftw    # its on-target test, from tests-8.0-<arch>
apk del fftw fftw-dev qnx-test-fftw
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/fftw.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/fftw.list
rm var/lib/manual/fftw.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild fftw` performs exactly the same steps (2–5) in one command.

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
URL=https://www.fftw.org/fftw-3.3.10.tar.gz
mkdir -p dist src/fftw
curl -fL -o dist/$(basename $URL) $URL
echo "56c932549852cddcfafdab3820b0200c7742675be92179e59e6215b340e26467  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/fftw --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/fftw; B=build/$QNX_ARCH/fftw; mkdir -p $B
```

```bash
do_configure() {
  local p
  for p in $FFTW_PRECS; do
    mkdir -p "$B/$p" && ( cd "$B/$p" && \
    CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" \
    CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
    CPPFLAGS="-D_QNX_SOURCE -I$STAGE$PREFIX/include" \
    CFLAGS="-O3 -D_QNX_SOURCE -I$STAGE$PREFIX/include" \
    LDFLAGS="-Wl,--push-state,--no-as-needed -lc -Wl,--pop-state -L$STAGE$PREFIX/lib -Wl,-rpath-link,$STAGE$PREFIX/lib -lm" \
    "$S/configure" --host="$QNX_TRIPLE" --prefix="$PREFIX" --enable-shared --disable-static \
      --enable-threads --disable-fortran --disable-doc $(fftw_simd $p) ) || return 1
  done
}
```

```bash
do_build() { local p; for p in $FFTW_PRECS; do make -C "$B/$p" -j"$(nproc)" || return 1; done; }
```

```bash
do_install() { local p; for p in $FFTW_PRECS; do make -C "$B/$p" DESTDIR="$STAGE" install; done; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- fftw: the Fastest Fourier Transform in the West -- discrete Fourier transforms (complex, real, r2r/DCT/DST) of any size and dimension.
- FFTW builds one precision per configure run, so the hooks configure, build and install it twice, as distributions do: double (libfftw3) and single (libfftw3f), each with its pthread library (libfftw3_threads, libfftw3f_threads). Autotools rather than FFTW's CMake build, which the FFTW authors call experimental and which has no NEON option. SIMD: on x86_64 SSE2/AVX/AVX2 codelets (FFTW checks the CPU with cpuid at run time, so the library still runs on a CPU without AVX); on aarch64 NEON, which FFTW has for single precision only. No Fortran wrappers (no Fortran compiler in the SDP). Shared libraries only.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH fftw`.

### 5. What it installs

25 files under `/usr/local`: 8 shared-library entries, 3 programs, 5 headers.

```
/usr/local/lib/libfftw3.so
/usr/local/lib/libfftw3.so.9
/usr/local/lib/libfftw3_threads.so
/usr/local/lib/libfftw3_threads.so.9
/usr/local/lib/libfftw3f.so
/usr/local/lib/libfftw3f.so.9
/usr/local/lib/libfftw3f_threads.so
/usr/local/lib/libfftw3f_threads.so.9
/usr/local/bin/fftw-wisdom
/usr/local/bin/fftw-wisdom-to-conf
/usr/local/bin/fftwf-wisdom
```

The complete list is `stage/<arch>/.manifest/fftw` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa fftw
ls dists/8.0/*/ | grep '^fftw-'
```

This writes `fftw-<ver>-r0.tar.gz` (run time) and `fftw-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add fftw
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/fftw-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/fftw-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_fftw`](../../tests/src/) links fftw the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-fftw`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-fftw
/usr/local/bin/qnx-port-tests/test_fftw
```

```
== fftw ==
  ok   ...
PASS fftw
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_fftw` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_fftw`](../../tests/src/) -- a program linked against fftw the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - cos(5): X[5] = N/2
   - cos(5): X[N-5] = N/2
   - cos(5): every other bin is 0
   - forward then backward = N * input
   - impulse: flat spectrum of 1
   - shifted impulse: X[N/4] = -i
   - r2c of constant 3: X[0] = 3N, others 0
   - DCT-II of constant: Y[0] = 2N*c
   - 2-D 8x8: spikes of 32 at (1,2) and (7,6)
   - N=1000 (FFTW_MEASURE) matches the naive DFT
   - fftw_init_threads
   - 4-thread plan matches the naive DFT
   - float: cos(17) + 0.25 -> X[17] = N/2, X[0] = N/4, rest 0
   - wisdom exported

Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> fftw`.

## Versions

This recipe builds **3.3.10**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add fftw=<version>`), sources in `ported/fftw/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates fftw`; port one with `scripts/port-update fftw`.

<details><summary>recipe.sh</summary>

```bash
# fftw: the Fastest Fourier Transform in the West -- discrete Fourier
# transforms (complex, real, r2r/DCT/DST) of any size and dimension.
VERSION=3.3.10
TARBALL=https://www.fftw.org/fftw-3.3.10.tar.gz
SHA256=56c932549852cddcfafdab3820b0200c7742675be92179e59e6215b340e26467
BUILDSYS=autotools
# FFTW builds one precision per configure run, so the hooks configure,
# build and install it twice, as distributions do: double (libfftw3) and
# single (libfftw3f), each with its pthread library (libfftw3_threads,
# libfftw3f_threads). Autotools rather than FFTW's CMake build, which the
# FFTW authors call experimental and which has no NEON option.
# SIMD: on x86_64 SSE2/AVX/AVX2 codelets (FFTW checks the CPU with cpuid at
# run time, so the library still runs on a CPU without AVX); on aarch64 NEON,
# which FFTW has for single precision only. No Fortran wrappers (no Fortran
# compiler in the SDP). Shared libraries only.
FFTW_PRECS="double float"
fftw_simd() {
  case "$QNX_ARCH:$1" in
    x86_64:double) echo "--enable-sse2 --enable-avx --enable-avx2" ;;
    x86_64:float)  echo "--enable-float --enable-sse --enable-sse2 --enable-avx --enable-avx2" ;;
    aarch64le:float) echo "--enable-float --enable-neon" ;;
    *:float) echo "--enable-float" ;;
  esac
}
do_configure() {
  local p
  for p in $FFTW_PRECS; do
    mkdir -p "$B/$p" && ( cd "$B/$p" && \
    CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" \
    CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
    CPPFLAGS="-D_QNX_SOURCE -I$STAGE$PREFIX/include" \
    CFLAGS="-O3 -D_QNX_SOURCE -I$STAGE$PREFIX/include" \
    LDFLAGS="-Wl,--push-state,--no-as-needed -lc -Wl,--pop-state -L$STAGE$PREFIX/lib -Wl,-rpath-link,$STAGE$PREFIX/lib -lm" \
    "$S/configure" --host="$QNX_TRIPLE" --prefix="$PREFIX" --enable-shared --disable-static \
      --enable-threads --disable-fortran --disable-doc $(fftw_simd $p) ) || return 1
  done
}
do_build() { local p; for p in $FFTW_PRECS; do make -C "$B/$p" -j"$(nproc)" || return 1; done; }
do_install() { local p; for p in $FFTW_PRECS; do make -C "$B/$p" DESTDIR="$STAGE" install; done; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
