# libvterm 0.3.3

> Abstract VT220/xterm/ECMA-48 terminal emulator library

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://www.leonerd.org.uk/code/libvterm/libvterm-0.3.3.tar.gz |
| SHA-256 | `09156f43dd2128bd347cbeebe50d9a571d32c64e0cf18d211197946aff7226e0` |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libvterm`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `libvterm` (programs, shared libraries, run-time data) and `libvterm-dev` (headers, pkg-config/CMake files, static libraries; needs `libvterm`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`libvterm`](https://openqnx.github.io/oss/package/x86_64/libvterm.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libvterm.html)) · [`libvterm-dev`](https://openqnx.github.io/oss/package/x86_64/libvterm-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libvterm-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add libvterm` (dependencies come along); one version: `apk add libvterm=<version>` | `apk del libvterm` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install libvterm` (dependencies first) | `oss-get remove libvterm` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`libvterm-dev` the same way, by its own name (`apk add libvterm-dev`, `oss-get install libvterm-dev`, by hand with its own file and list `libvterm-dev.list`); remove it before `libvterm`, which it needs.

```sh
# on the QNX target, as root
apk add libvterm libvterm-dev
apk add qnx-test-libvterm && /usr/local/bin/qnx-port-tests/test_libvterm    # its on-target test, from tests-8.0-<arch>
apk del libvterm libvterm-dev qnx-test-libvterm
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/libvterm.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/libvterm.list
rm var/lib/manual/libvterm.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libvterm` performs exactly the same steps (2–5) in one command.

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
URL=https://www.leonerd.org.uk/code/libvterm/libvterm-0.3.3.tar.gz
mkdir -p dist src/libvterm
curl -fL -o dist/$(basename $URL) $URL
echo "09156f43dd2128bd347cbeebe50d9a571d32c64e0cf18d211197946aff7226e0  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/libvterm --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/libvterm < ports/libvterm/patches/0001-vterm-dump-xopen-source.patch
```

**[`0001-vterm-dump-xopen-source.patch`](patches/0001-vterm-dump-xopen-source.patch)** — Give vterm-dump's _XOPEN_SOURCE a version QNX accepts.

> vterm-dump.c defines _XOPEN_SOURCE with no value to get getopt(3). QNX's
> <sys/platform.h> accepts only the X/Open versions it knows (500, 600, 700)
> and stops with "#error This version of XOPEN_SOURCE is not supported".
> Give it the value vterm-ctrl.c already uses (600, which declares getopt).

Touches: `bin/vterm-dump.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/libvterm; B=build/$QNX_ARCH/libvterm; mkdir -p $B
```

```bash
do_build() {
  rm -rf "$B" && mkdir -p "$B" && cd "$B" || return 1
  local f
  for f in "$S"/src/*.c; do _vt_cc -fPIC -c "$f" -o "$(basename "${f%.c}").o" || return 1; done
  qcc -V$QCC_TARGET -shared -Wl,-soname,libvterm.so.0 -o libvterm.so.0.0.0 ./*.o || return 1
  ln -sf libvterm.so.0.0.0 libvterm.so.0 && ln -sf libvterm.so.0 libvterm.so || return 1
  for f in unterm vterm-ctrl vterm-dump; do _vt_cc -o "$f" "$S/bin/$f.c" -L. -lvterm || return 1; done
}
```

```bash
do_install() {
  local L="$STAGE$PREFIX/lib"
  install -d "$L/pkgconfig" "$STAGE$PREFIX/include" "$STAGE$PREFIX/bin" &&
  install -m644 "$S"/include/*.h "$STAGE$PREFIX/include/" &&
  install -m755 "$B/libvterm.so.0.0.0" "$L/" && ln -sf libvterm.so.0.0.0 "$L/libvterm.so.0" && ln -sf libvterm.so.0 "$L/libvterm.so" &&
  install -m755 "$B/unterm" "$B/vterm-ctrl" "$B/vterm-dump" "$STAGE$PREFIX/bin/" &&
  sed -e "s,@INCDIR@,$PREFIX/include," -e "s,@LIBDIR@,$PREFIX/lib," -e "s,@VERSION@,$VERSION," "$S/vterm.pc.in" > "$L/pkgconfig/vterm.pc"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- libvterm: an abstract VT220/xterm/ECMA-48 terminal emulator library (the one vim and neovim embed), with its vterm-ctrl/vterm-dump/unterm tools.
- The Makefile drives everything through the build machine's libtool, which knows nothing of the QNX cross compiler. The release tarball already has the generated tables (src/encoding/*.inc, src/fullwidth.inc), so the library is just src/*.c: compiled position-independent with qcc and linked into what libtool would have made from -version-info 0:0:0 -- libvterm.so.0.0.0, soname libvterm.so.0.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libvterm`.

### 5. What it installs

9 files under `/usr/local`: 3 shared-library entries, 3 programs, 2 headers.

```
/usr/local/lib/libvterm.so
/usr/local/lib/libvterm.so.0
/usr/local/lib/libvterm.so.0.0.0
/usr/local/bin/unterm
/usr/local/bin/vterm-ctrl
/usr/local/bin/vterm-dump
```

The complete list is `stage/<arch>/.manifest/libvterm` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libvterm
ls dists/8.0/*/ | grep '^libvterm-'
```

This writes `libvterm-<ver>-r0.tar.gz` (run time) and `libvterm-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libvterm
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libvterm-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libvterm-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libvterm`](../../tests/src/) links libvterm the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libvterm`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libvterm
/usr/local/bin/qnx-port-tests/test_libvterm
```

```
== libvterm ==
  ok   ...
PASS libvterm
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libvterm` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libvterm`](../../tests/src/) -- a program linked against libvterm the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - vterm_new(25, 80)
   - vterm_get_size: 25x80
   - text at row 0: \"Hello\"
   - SGR 1;31: bold, red (palette index 1)
   - SGR 0 resets the attributes
   - CUP ESC[2;5H: X at row 1 col 4, cursor after it
   - UTF-8 \"\\u00e9\" is one cell
   - CJK \"\\u4e2d\" is two cells wide
   - vterm_screen_get_text of row 0: \"HelloRz\"
   - ED 2 (ESC[2J) clears the screen
   - keyboard: Up -> ESC [ A
   - keyboard: Ctrl-A -> 0x01
   - keyboard with DECCKM: Up -> ESC O A
   - DA query ESC[c answered with ESC[?...c

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libvterm 0.3.3-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/aarch64le/libvterm-0.3.3-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libvterm 0.3.3-r0 | ✅ PASS | 2026-11-04 | [log](../../tests/logs/x86_64/libvterm-0.3.3-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.3.3**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libvterm=<version>`), sources in `ported/libvterm/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libvterm`; port one with `scripts/port-update libvterm`.

<details><summary>recipe.sh</summary>

```bash
# libvterm: an abstract VT220/xterm/ECMA-48 terminal emulator library (the
# one vim and neovim embed), with its vterm-ctrl/vterm-dump/unterm tools.
VERSION=0.3.3
TARBALL=https://www.leonerd.org.uk/code/libvterm/libvterm-0.3.3.tar.gz
SHA256=09156f43dd2128bd347cbeebe50d9a571d32c64e0cf18d211197946aff7226e0
BUILDSYS=custom
# The Makefile drives everything through the build machine's libtool, which
# knows nothing of the QNX cross compiler. The release tarball already has the
# generated tables (src/encoding/*.inc, src/fullwidth.inc), so the library is
# just src/*.c: compiled position-independent with qcc and linked into what
# libtool would have made from -version-info 0:0:0 -- libvterm.so.0.0.0,
# soname libvterm.so.0.
_vt_cc() { qcc -V$QCC_TARGET -D_QNX_SOURCE -O2 -Wall -std=c99 -I"$S/include" "$@"; }
do_build() {
  rm -rf "$B" && mkdir -p "$B" && cd "$B" || return 1
  local f
  for f in "$S"/src/*.c; do _vt_cc -fPIC -c "$f" -o "$(basename "${f%.c}").o" || return 1; done
  qcc -V$QCC_TARGET -shared -Wl,-soname,libvterm.so.0 -o libvterm.so.0.0.0 ./*.o || return 1
  ln -sf libvterm.so.0.0.0 libvterm.so.0 && ln -sf libvterm.so.0 libvterm.so || return 1
  for f in unterm vterm-ctrl vterm-dump; do _vt_cc -o "$f" "$S/bin/$f.c" -L. -lvterm || return 1; done
}
do_install() {
  local L="$STAGE$PREFIX/lib"
  install -d "$L/pkgconfig" "$STAGE$PREFIX/include" "$STAGE$PREFIX/bin" &&
  install -m644 "$S"/include/*.h "$STAGE$PREFIX/include/" &&
  install -m755 "$B/libvterm.so.0.0.0" "$L/" && ln -sf libvterm.so.0.0.0 "$L/libvterm.so.0" && ln -sf libvterm.so.0 "$L/libvterm.so" &&
  install -m755 "$B/unterm" "$B/vterm-ctrl" "$B/vterm-dump" "$STAGE$PREFIX/bin/" &&
  sed -e "s,@INCDIR@,$PREFIX/include," -e "s,@LIBDIR@,$PREFIX/lib," -e "s,@VERSION@,$VERSION," "$S/vterm.pc.in" > "$L/pkgconfig/vterm.pc"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
