# wlr-protocols 0-git-20250702

> Wayland protocol extensions from wlroots (XML files)

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://gitlab.freedesktop.org/wlroots/wlr-protocols.git |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_wlr_protocols`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `wlr-protocols` (programs, shared libraries, run-time data) and `wlr-protocols-dev` (headers, pkg-config/CMake files, static libraries; needs `wlr-protocols`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`wlr-protocols`](https://openqnx.github.io/oss/package/x86_64/wlr-protocols.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/wlr-protocols.html)) · [`wlr-protocols-dev`](https://openqnx.github.io/oss/package/x86_64/wlr-protocols-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/wlr-protocols-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add wlr-protocols` (dependencies come along); one version: `apk add wlr-protocols=<version>` | `apk del wlr-protocols` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install wlr-protocols` (dependencies first) | `oss-get remove wlr-protocols` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`wlr-protocols-dev` the same way, by its own name (`apk add wlr-protocols-dev`, `oss-get install wlr-protocols-dev`, by hand with its own file and list `wlr-protocols-dev.list`); remove it before `wlr-protocols`, which it needs.

```sh
# on the QNX target, as root
apk add wlr-protocols wlr-protocols-dev
apk add qnx-test-wlr-protocols && /usr/local/bin/qnx-port-tests/test_wlr_protocols    # its on-target test
apk del wlr-protocols wlr-protocols-dev qnx-test-wlr-protocols
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/wlr-protocols.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/wlr-protocols.list
rm var/lib/manual/wlr-protocols.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild wlr-protocols` performs exactly the same steps (2–5) in one command.

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
    https://gitlab.freedesktop.org/wlroots/wlr-protocols.git src/wlr-protocols
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/wlr-protocols; B=build/$QNX_ARCH/wlr-protocols; mkdir -p $B
```

```bash
do_build() { :; }
```

```bash
do_install() { make -C "$S" DESTDIR="$STAGE" PREFIX="$PREFIX" install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- wlroots' Wayland protocol extensions (XML; data only). Version as in the image.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH wlr-protocols`.

### 5. What it installs

11 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/wlr-protocols` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa wlr-protocols
ls dists/8.0/*/ | grep '^wlr-protocols-'
```

This writes `wlr-protocols-<ver>-r0.tar.gz` (run time) and `wlr-protocols-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add wlr-protocols
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/wlr-protocols-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/wlr-protocols-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_wlr_protocols`](../../tests/src/) links wlr-protocols the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-wlr-protocols`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-wlr-protocols
/usr/local/bin/qnx-port-tests/test_wlr_protocols
```

```
== wlr-protocols ==
  ok   ...
PASS wlr-protocols
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_wlr_protocols` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_wlr_protocols`](../../tests/src/) -- a program linked against wlr-protocols the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - share/wlr-protocols installed
   - every protocol XML file parses (10 or more)
   - each is a protocol with interfaces
   - layer shell: zwlr_layer_shell_v1.get_layer_surface
   - layers background 0, bottom 1, top 2, overlay 3
   - layer surface: set_anchor, anchors top 1 .. right 8, configure
   - screencopy: capture_output
   - output management, foreign toplevel management

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | wlr-protocols 0_git20250702-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/wlr-protocols-0_git20250702-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | wlr-protocols 0_git20250702-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/wlr-protocols-0_git20250702-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | wlr-protocols 0_git20250702-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/wlr-protocols-0_git20250702-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | wlr-protocols 0_git20250702-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/wlr-protocols-0_git20250702-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0-git-20250702**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add wlr-protocols=<version>`), sources in `ported/wlr-protocols/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates wlr-protocols`; port one with `scripts/port-update wlr-protocols`.

<details><summary>recipe.sh</summary>

```bash
# wlroots' Wayland protocol extensions (XML; data only). Version as in the image.
VERSION=0-git-20250702; GIT=https://gitlab.freedesktop.org/wlroots/wlr-protocols.git; COMMIT=a5028afbe4a1cf0daf020c4104c1565a09d6e58a
BUILDSYS=custom
do_build() { :; }
do_install() { make -C "$S" DESTDIR="$STAGE" PREFIX="$PREFIX" install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
