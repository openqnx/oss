# muslflt 1.0-git-20240802

> musl's libm and float conversion routines as a separate library

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/qnx-ports/muslflt.git |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_muslflt`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add muslflt
apk add muslflt-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-muslflt && /usr/local/bin/qnx-port-tests/test_muslflt    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install muslflt`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild muslflt` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 --recurse-submodules --shallow-submodules \
    https://github.com/qnx-ports/muslflt.git src/muslflt
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/muslflt < ports/muslflt/patches/0001-aarch64-qnx-rounding-mode-values.patch
```

**[`0001-aarch64-qnx-rounding-mode-values.patch`](patches/0001-aarch64-qnx-rounding-mode-values.patch)** — QNX's <fenv.h> values (the FPCR RMode field, not shifted into place):

Touches: `fenv/aarch64le/bits/fenv.h`, `fenv/aarch64le/fenv.s`

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/muslflt; B=build/$QNX_ARCH/muslflt; mkdir -p $B
```

```bash
do_build() {
  local cpu=$QNX_ARCH dirs="string complex fenv math qnx" d f base
  declare -A pick
  for d in $dirs; do for f in "$S/$d"/*.c; do [ -e "$f" ] && pick[$(basename "${f%.*}")]="$f"; done; done
  for d in fenv/$cpu math/$cpu; do for f in "$S/$d"/*.[csS]; do [ -e "$f" ] && pick[$(basename "${f%.*}")]="$f"; done; done
  mkdir -p "$B/obj"; local inc="-I$S/fenv/$cpu -I$S/math/$cpu -I$S/math -I$S/fenv -I$S/string -I$S/complex -I$S/qnx"
  local objs=() n=0
  for base in "${!pick[@]}"; do
    f=${pick[$base]}
    qcc -V$QCC_TARGET -c -O2 -fPIC -D_QNX_SOURCE -Wno-parentheses -Werror=implicit-function-declaration \
        -Wno-unused-but-set-variable $inc "$f" -o "$B/obj/$base.o" || return 1
    objs+=("$B/obj/$base.o"); n=$((n+1))
  done
  echo "muslflt: $n objects"
  # -Bsymbolic: musl's functions call each other (snprintf -> vsnprintf,
  # cexp -> exp, printf's %a -> frexpl). Through the PLT those calls bind to the
  # first definition in the process, and QNX's libc/libm are always loaded
  # before this library (libc.so is the first DT_NEEDED of every executable),
  # so muslflt's snprintf ran QNX libc's vsnprintf. Bind them inside the library.
  qcc -V$QCC_TARGET -shared -Wl,-soname,libmuslflt.so.1 -Wl,-Bsymbolic -o "$B/libmuslflt.so.1" "${objs[@]}" -lm || return 1
  "$QNX_HOST/usr/bin/nto${QNX_CPU}-ar" rcs "$B/libmuslflt.a" "${objs[@]}"
}
```

```bash
do_install() {
  install -d "$STAGE$PREFIX/lib" && install -m755 "$B/libmuslflt.so.1" "$STAGE$PREFIX/lib/" && \
  ln -sfn libmuslflt.so.1 "$STAGE$PREFIX/lib/libmuslflt.so" && install -m644 "$B/libmuslflt.a" "$STAGE$PREFIX/lib/"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- muslflt: musl's libm (math.h, fenv.h, complex.h) and its string<->float conversions, packaged by qnx-ports so QNX programs can get the float behaviour of other UNIX systems by linking -lmuslflt. A qnx-ports project (not an upstream release); pinned commit.
- qnx-ports builds it with QNX recursive make from a list of source directories. The same here, explicitly: every .c/.s/.S in string, complex, fenv, math and qnx, where a file in an architecture directory (math/<cpu>, fenv/<cpu>) replaces the generic file of the same name -- musl's rule.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH muslflt`.

### 5. What it installs

3 files under `/usr/local`: 2 shared-library entries.

```
/usr/local/lib/libmuslflt.so
/usr/local/lib/libmuslflt.so.1
```

The complete list is `stage/<arch>/.manifest/muslflt` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa muslflt
ls dists/8.0/*/ | grep '^muslflt-'
```

This writes `muslflt-<ver>-r0.tar.gz` (run time) and `muslflt-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add muslflt
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/muslflt-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/muslflt-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_muslflt`](../../tests/src/) links muslflt the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-muslflt`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-muslflt
/usr/local/bin/qnx-port-tests/test_muslflt
```

```
== muslflt ==
  ok   ...
PASS muslflt
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_muslflt` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_muslflt`](../../tests/src/) -- a program linked against muslflt the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - dlopen libmuslflt.so.1
   - sqrt(2) correctly rounded
   - exp(1)
   - log(10)
   - sin(1), cos(1)
   - sin(1e22) (Payne-Hanek reduction)
   - pow(2,10)=1024, pow(-8,1/3)=NaN
   - cbrt(-27) = -3, hypot(3,4) = 5
   - tgamma(5), lgamma(1/2) = log(sqrt(pi))
   - erf(0.5)
   - fma(0.1, 10, -1) is fused (5.55e-17)
   - rint/round/trunc/floor
   - fmod and remainder
   - special values
   - fesetround(FE_UPWARD)
   - rint follows the rounding mode
   - cexp(i*pi) = -1 (Euler)
   - csqrt(-4) = 2i
   - cabs(3+4i) = 5
   - snprintf float formats
   - snprintf %.20f
   - snprintf ties-to-even and inf
   - strtod hex float, end pointer
   - strtod near DBL_MIN (correct rounding)
   - strtod 2^53+1 rounds to even
   - ... and 5 more

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | muslflt 1.0_git20240802-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/muslflt-1.0_git20240802-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | muslflt 1.0_git20240802-r2 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/muslflt-1.0_git20240802-r2.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | muslflt 1.0_git20240802-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/muslflt-1.0_git20240802-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | muslflt 1.0_git20240802-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/muslflt-1.0_git20240802-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.0-git-20240802**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add muslflt=<version>`), sources in `ported/muslflt/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates muslflt`; port one with `scripts/port-update muslflt`.

<details><summary>recipe.sh</summary>

```bash
# muslflt: musl's libm (math.h, fenv.h, complex.h) and its string<->float
# conversions, packaged by qnx-ports so QNX programs can get the float
# behaviour of other UNIX systems by linking -lmuslflt. A qnx-ports project
# (not an upstream release); pinned commit.
VERSION=1.0-git-20240802; GIT=https://github.com/qnx-ports/muslflt.git; COMMIT=37ac677512d09b83e26533807d2fb330d4fcabd6
BUILDSYS=custom
# qnx-ports builds it with QNX recursive make from a list of source
# directories. The same here, explicitly: every .c/.s/.S in string, complex,
# fenv, math and qnx, where a file in an architecture directory (math/<cpu>,
# fenv/<cpu>) replaces the generic file of the same name -- musl's rule.
do_build() {
  local cpu=$QNX_ARCH dirs="string complex fenv math qnx" d f base
  declare -A pick
  for d in $dirs; do for f in "$S/$d"/*.c; do [ -e "$f" ] && pick[$(basename "${f%.*}")]="$f"; done; done
  for d in fenv/$cpu math/$cpu; do for f in "$S/$d"/*.[csS]; do [ -e "$f" ] && pick[$(basename "${f%.*}")]="$f"; done; done
  mkdir -p "$B/obj"; local inc="-I$S/fenv/$cpu -I$S/math/$cpu -I$S/math -I$S/fenv -I$S/string -I$S/complex -I$S/qnx"
  local objs=() n=0
  for base in "${!pick[@]}"; do
    f=${pick[$base]}
    qcc -V$QCC_TARGET -c -O2 -fPIC -D_QNX_SOURCE -Wno-parentheses -Werror=implicit-function-declaration \
        -Wno-unused-but-set-variable $inc "$f" -o "$B/obj/$base.o" || return 1
    objs+=("$B/obj/$base.o"); n=$((n+1))
  done
  echo "muslflt: $n objects"
  # -Bsymbolic: musl's functions call each other (snprintf -> vsnprintf,
  # cexp -> exp, printf's %a -> frexpl). Through the PLT those calls bind to the
  # first definition in the process, and QNX's libc/libm are always loaded
  # before this library (libc.so is the first DT_NEEDED of every executable),
  # so muslflt's snprintf ran QNX libc's vsnprintf. Bind them inside the library.
  qcc -V$QCC_TARGET -shared -Wl,-soname,libmuslflt.so.1 -Wl,-Bsymbolic -o "$B/libmuslflt.so.1" "${objs[@]}" -lm || return 1
  "$QNX_HOST/usr/bin/nto${QNX_CPU}-ar" rcs "$B/libmuslflt.a" "${objs[@]}"
}
do_install() {
  install -d "$STAGE$PREFIX/lib" && install -m755 "$B/libmuslflt.so.1" "$STAGE$PREFIX/lib/" && \
  ln -sfn libmuslflt.so.1 "$STAGE$PREFIX/lib/libmuslflt.so" && install -m644 "$B/libmuslflt.a" "$STAGE$PREFIX/lib/"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
