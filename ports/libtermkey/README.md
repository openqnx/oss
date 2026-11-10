# libtermkey 0.22

> Keyboard entry processing library for terminal programs

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://www.leonerd.org.uk/code/libtermkey/libtermkey-0.22.tar.gz |
| SHA-256 | `6945bd3c4aaa83da83d80a045c5563da4edd7d0374c62c0d35aec09eb3014600` |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | [`unibilium`](../unibilium/) |
| Run-time dependencies | [`unibilium`](../unibilium/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libtermkey`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `libtermkey` (programs, shared libraries, run-time data) and `libtermkey-dev` (headers, pkg-config/CMake files, static libraries; needs `libtermkey`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`libtermkey`](https://openqnx.github.io/oss/package/x86_64/libtermkey.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libtermkey.html)) · [`libtermkey-dev`](https://openqnx.github.io/oss/package/x86_64/libtermkey-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libtermkey-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add libtermkey` (dependencies come along); one version: `apk add libtermkey=<version>` | `apk del libtermkey` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install libtermkey` (dependencies first) | `oss-get remove libtermkey` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`libtermkey-dev` the same way, by its own name (`apk add libtermkey-dev`, `oss-get install libtermkey-dev`, by hand with its own file and list `libtermkey-dev.list`); remove it before `libtermkey`, which it needs.

```sh
# on the QNX target, as root
apk add libtermkey libtermkey-dev
apk add qnx-test-libtermkey && /usr/local/bin/qnx-port-tests/test_libtermkey    # its on-target test, from tests-8.0-<arch>
apk del libtermkey libtermkey-dev qnx-test-libtermkey
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/libtermkey.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/libtermkey.list
rm var/lib/manual/libtermkey.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libtermkey` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (1 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild unibilium
```

### 2. Get the upstream source

```bash
URL=https://www.leonerd.org.uk/code/libtermkey/libtermkey-0.22.tar.gz
mkdir -p dist src/libtermkey
curl -fL -o dist/$(basename $URL) $URL
echo "6945bd3c4aaa83da83d80a045c5563da4edd7d0374c62c0d35aec09eb3014600  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/libtermkey --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/libtermkey; B=build/$QNX_ARCH/libtermkey; mkdir -p $B
```

```bash
do_build() {
  rm -rf "$B" && mkdir -p "$B" && cd "$B" || return 1
  local f
  for f in termkey driver-csi driver-ti; do _tk_cc -fPIC -c "$S/$f.c" -o "$f.o" || return 1; done
  qcc -V$QCC_TARGET -shared -Wl,-soname,libtermkey.so.1 -o libtermkey.so.1.14.2 ./*.o \
    -L"$STAGE_SYSROOT$PREFIX/lib" -lunibilium || return 1
  ln -sf libtermkey.so.1.14.2 libtermkey.so.1 && ln -sf libtermkey.so.1 libtermkey.so || return 1
  # demo: prints each key pressed (the upstream example program)
  _tk_cc -I"$S" -o termkey-demo "$S/demo.c" -L. -ltermkey -L"$STAGE_SYSROOT$PREFIX/lib" -lunibilium
}
```

```bash
do_install() {
  local L="$STAGE$PREFIX/lib"
  install -d "$L/pkgconfig" "$STAGE$PREFIX/include" "$STAGE$PREFIX/bin" &&
  install -m644 "$S/termkey.h" "$STAGE$PREFIX/include/" &&
  install -m755 "$B/libtermkey.so.1.14.2" "$L/" && ln -sf libtermkey.so.1.14.2 "$L/libtermkey.so.1" && ln -sf libtermkey.so.1 "$L/libtermkey.so" &&
  install -m755 "$B/termkey-demo" "$STAGE$PREFIX/bin/" &&
  LIBDIR=$PREFIX/lib INCDIR=$PREFIX/include VERSION=$VERSION sh "$S/termkey.pc.sh" > "$L/pkgconfig/termkey.pc"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- libtermkey: keyboard entry processing for terminal programs (key names, modifiers, CSI sequences, mouse reports), reading terminfo through unibilium. (Upstream has folded it into libtickit; it is still the library older neovim/vim plugins and tools link.)
- The Makefile drives everything through the build machine's libtool, which knows nothing of the QNX cross compiler: the three sources are compiled position-independent with qcc and linked into what libtool would have made from -version-info 15:2:14 -- libtermkey.so.1.14.2, soname libtermkey.so.1. HAVE_UNIBILIUM as the Makefile sets it when pkg-config finds unibilium.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libtermkey`.

### 5. What it installs

6 files under `/usr/local`: 3 shared-library entries, 1 programs, 1 headers.

```
/usr/local/lib/libtermkey.so
/usr/local/lib/libtermkey.so.1
/usr/local/lib/libtermkey.so.1.14.2
/usr/local/bin/termkey-demo
```

The complete list is `stage/<arch>/.manifest/libtermkey` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libtermkey
ls dists/8.0/*/ | grep '^libtermkey-'
```

This writes `libtermkey-<ver>-r0.tar.gz` (run time) and `libtermkey-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libtermkey
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libtermkey-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libtermkey-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libtermkey`](../../tests/src/) links libtermkey the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libtermkey`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libtermkey
/usr/local/bin/qnx-port-tests/test_libtermkey
```

```
== libtermkey ==
  ok   ...
PASS libtermkey
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libtermkey` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libtermkey`](../../tests/src/) -- a program linked against libtermkey the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - termkey_new_abstract(\"vt100\", UTF8)
   - \"a\" -> codepoint 'a'
   - UTF-8 \"\\u00e9\" -> U+00E9
   - 0x01 -> Ctrl-a (\"<C-a>\")
   - ESC[A -> Up
   - ESC O P -> F1
   - ESC[5~ -> PageUp
   - ESC x -> Alt-x (\"<M-x>\")
   - X10 mouse report -> press, button 1, line 3, col 2
   - termkey_keyname2sym(\"PageUp\")
   - a lone ESC: wait for more (TERMKEY_RES_AGAIN)
   - forced: the lone ESC is the Escape key
   - buffer empty afterwards

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libtermkey 0.22-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/aarch64le/libtermkey-0.22-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libtermkey 0.22-r0 | ✅ PASS | 2026-11-04 | [log](../../tests/logs/x86_64/libtermkey-0.22-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.22**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libtermkey=<version>`), sources in `ported/libtermkey/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libtermkey`; port one with `scripts/port-update libtermkey`.

<details><summary>recipe.sh</summary>

```bash
# libtermkey: keyboard entry processing for terminal programs (key names,
# modifiers, CSI sequences, mouse reports), reading terminfo through
# unibilium. (Upstream has folded it into libtickit; it is still the library
# older neovim/vim plugins and tools link.)
VERSION=0.22
TARBALL=https://www.leonerd.org.uk/code/libtermkey/libtermkey-0.22.tar.gz
SHA256=6945bd3c4aaa83da83d80a045c5563da4edd7d0374c62c0d35aec09eb3014600
DEPS="unibilium"
BUILDSYS=custom
# The Makefile drives everything through the build machine's libtool, which
# knows nothing of the QNX cross compiler: the three sources are compiled
# position-independent with qcc and linked into what libtool would have made
# from -version-info 15:2:14 -- libtermkey.so.1.14.2, soname libtermkey.so.1.
# HAVE_UNIBILIUM as the Makefile sets it when pkg-config finds unibilium.
_tk_cc() { qcc -V$QCC_TARGET -D_QNX_SOURCE -O2 -Wall -std=c99 -DHAVE_UNIBILIUM -I"$STAGE_SYSROOT$PREFIX/include" "$@"; }
do_build() {
  rm -rf "$B" && mkdir -p "$B" && cd "$B" || return 1
  local f
  for f in termkey driver-csi driver-ti; do _tk_cc -fPIC -c "$S/$f.c" -o "$f.o" || return 1; done
  qcc -V$QCC_TARGET -shared -Wl,-soname,libtermkey.so.1 -o libtermkey.so.1.14.2 ./*.o \
    -L"$STAGE_SYSROOT$PREFIX/lib" -lunibilium || return 1
  ln -sf libtermkey.so.1.14.2 libtermkey.so.1 && ln -sf libtermkey.so.1 libtermkey.so || return 1
  # demo: prints each key pressed (the upstream example program)
  _tk_cc -I"$S" -o termkey-demo "$S/demo.c" -L. -ltermkey -L"$STAGE_SYSROOT$PREFIX/lib" -lunibilium
}
do_install() {
  local L="$STAGE$PREFIX/lib"
  install -d "$L/pkgconfig" "$STAGE$PREFIX/include" "$STAGE$PREFIX/bin" &&
  install -m644 "$S/termkey.h" "$STAGE$PREFIX/include/" &&
  install -m755 "$B/libtermkey.so.1.14.2" "$L/" && ln -sf libtermkey.so.1.14.2 "$L/libtermkey.so.1" && ln -sf libtermkey.so.1 "$L/libtermkey.so" &&
  install -m755 "$B/termkey-demo" "$STAGE$PREFIX/bin/" &&
  LIBDIR=$PREFIX/lib INCDIR=$PREFIX/include VERSION=$VERSION sh "$S/termkey.pc.sh" > "$L/pkgconfig/termkey.pc"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
