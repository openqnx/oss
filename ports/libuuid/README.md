# libuuid 2.42.4

> Library for generating and handling UUIDs (from util-linux)

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://mirrors.edge.kernel.org/pub/linux/utils/util-linux/v2.42/util-linux-2.42.4.tar.xz |
| SHA-256 | `fbd62a100ab7bb8746ba0661255c3c48185b1e9021507c624da01fbc696330ec` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libuuid`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `libuuid` (programs, shared libraries, run-time data) and `libuuid-dev` (headers, pkg-config/CMake files, static libraries; needs `libuuid`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`libuuid`](https://openqnx.github.io/oss/package/x86_64/libuuid.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libuuid.html)) · [`libuuid-dev`](https://openqnx.github.io/oss/package/x86_64/libuuid-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libuuid-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add libuuid` (dependencies come along); one version: `apk add libuuid=<version>` | `apk del libuuid` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install libuuid` (dependencies first) | `oss-get remove libuuid` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`libuuid-dev` the same way, by its own name (`apk add libuuid-dev`, `oss-get install libuuid-dev`, by hand with its own file and list `libuuid-dev.list`); remove it before `libuuid`, which it needs.

```sh
# on the QNX target, as root
apk add libuuid libuuid-dev
apk add qnx-test-libuuid && /usr/local/bin/qnx-port-tests/test_libuuid    # its on-target test, from tests-8.0-<arch>
apk del libuuid libuuid-dev qnx-test-libuuid
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/libuuid.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/libuuid.list
rm var/lib/manual/libuuid.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libuuid` performs exactly the same steps (2–5) in one command.

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
URL=https://mirrors.edge.kernel.org/pub/linux/utils/util-linux/v2.42/util-linux-2.42.4.tar.xz
mkdir -p dist src/libuuid
curl -fL -o dist/$(basename $URL) $URL
echo "fbd62a100ab7bb8746ba0661255c3c48185b1e9021507c624da01fbc696330ec  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/libuuid --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/libuuid; B=build/$QNX_ARCH/libuuid; mkdir -p $B
```

```bash
do_install() {
  make -C "$B" DESTDIR="$STAGE" install && rm -f "$STAGE$PREFIX/share/man/man5/terminal-colors.d.5" &&
  rmdir "$STAGE$PREFIX/share/man/man5" 2>/dev/null; true
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- libuuid from util-linux, the maintained one: the standalone libuuid 1.0.3 on SourceForge is a 2014 snapshot of this same code that has had no fixes since. Only the library is built (--disable-all-programs --enable-libuuid): libuuid.so.1, <uuid/uuid.h> and uuid.pc, the same ABI as on Linux.
- util-linux installs terminal-colors.d(5) whatever is enabled; it describes programs this package does not have.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libuuid`.

### 5. What it installs

17 files under `/usr/local`: 3 shared-library entries, 1 headers.

```
/usr/local/lib/libuuid.so
/usr/local/lib/libuuid.so.1
/usr/local/lib/libuuid.so.1.3.0
```

The complete list is `stage/<arch>/.manifest/libuuid` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libuuid
ls dists/8.0/*/ | grep '^libuuid-'
```

This writes `libuuid-<ver>-r0.tar.gz` (run time) and `libuuid-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libuuid
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libuuid-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libuuid-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libuuid`](../../tests/src/) links libuuid the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libuuid`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libuuid
/usr/local/bin/qnx-port-tests/test_libuuid
```

```
== libuuid ==
  ok   ...
PASS libuuid
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libuuid` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libuuid`](../../tests/src/) -- a program linked against libuuid the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - uuid_parse (the RFC 4122 DNS namespace)
   - it is a version 1, DCE variant UUID
   - uuid_unparse_upper
   - uuid_parse rejects a short or malformed string
   - uuid_get_template(\"dns\") is the DNS namespace parsed above
   - uuid_generate_md5(dns, www.example.com) = Python uuid3
   - uuid_generate_sha1(dns, www.example.com) = Python uuid5
   - 1000 uuid_generate_random(): all version 4, all distinct
   - uuid_generate_time: version 1, embedded time is now
   - a second v1 UUID is not older
   - uuid_generate_time_v7: version 7, time is now
   - uuid_copy + uuid_compare
   - uuid_clear -> uuid_is_null

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libuuid 2.42.4-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/libuuid-2.42.4-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libuuid 2.42.4-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/libuuid-2.42.4-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.42.4**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libuuid=<version>`), sources in `ported/libuuid/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libuuid`; port one with `scripts/port-update libuuid`.

<details><summary>recipe.sh</summary>

```bash
# libuuid from util-linux, the maintained one: the standalone libuuid 1.0.3
# on SourceForge is a 2014 snapshot of this same code that has had no fixes
# since. Only the library is built (--disable-all-programs --enable-libuuid):
# libuuid.so.1, <uuid/uuid.h> and uuid.pc, the same ABI as on Linux.
VERSION=2.42.4
TARBALL=https://mirrors.edge.kernel.org/pub/linux/utils/util-linux/v2.42/util-linux-2.42.4.tar.xz
SHA256=fbd62a100ab7bb8746ba0661255c3c48185b1e9021507c624da01fbc696330ec
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --disable-all-programs --enable-libuuid --enable-shared --disable-static
  --disable-nls --disable-bash-completion --disable-asciidoc --disable-poman --without-python
  --without-systemd --without-udev --without-ncurses --without-ncursesw --without-readline
  --without-libmagic --without-cap-ng --without-btrfs --without-selinux --without-audit
  --without-user --without-econf --disable-liblastlog2 )
# util-linux installs terminal-colors.d(5) whatever is enabled; it describes
# programs this package does not have.
do_install() {
  make -C "$B" DESTDIR="$STAGE" install && rm -f "$STAGE$PREFIX/share/man/man5/terminal-colors.d.5" &&
  rmdir "$STAGE$PREFIX/share/man/man5" 2>/dev/null; true
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
