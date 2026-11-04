# openresolv 3.17.4

> resolvconf: manages resolv.conf from several DNS information sources

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/NetworkConfiguration/openresolv/releases/download/v3.17.4/openresolv-3.17.4.tar.xz |
| SHA-256 | `901d83a15520b80e117aeca8cb6f5b70ceb205ba050a638d45cebcf304df729b` |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_openresolv`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `openresolv` (programs, shared libraries, run-time data) and `openresolv-dev` (headers, pkg-config/CMake files, static libraries; needs `openresolv`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`openresolv`](https://openqnx.github.io/oss/package/x86_64/openresolv.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/openresolv.html)) · [`openresolv-dev`](https://openqnx.github.io/oss/package/x86_64/openresolv-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/openresolv-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add openresolv` (dependencies come along); one version: `apk add openresolv=<version>` | `apk del openresolv` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install openresolv` (dependencies first) | `oss-get remove openresolv` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`openresolv-dev` the same way, by its own name (`apk add openresolv-dev`, `oss-get install openresolv-dev`, by hand with its own file and list `openresolv-dev.list`); remove it before `openresolv`, which it needs.

```sh
# on the QNX target, as root
apk add openresolv openresolv-dev
apk add qnx-test-openresolv && /usr/local/bin/qnx-port-tests/test_openresolv    # its on-target test, from tests-8.0-<arch>-m-z
apk del openresolv openresolv-dev qnx-test-openresolv
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/openresolv.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/openresolv.list
rm var/lib/manual/openresolv.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild openresolv` performs exactly the same steps (2–5) in one command.

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
URL=https://github.com/NetworkConfiguration/openresolv/releases/download/v3.17.4/openresolv-3.17.4.tar.xz
mkdir -p dist src/openresolv
curl -fL -o dist/$(basename $URL) $URL
echo "901d83a15520b80e117aeca8cb6f5b70ceb205ba050a638d45cebcf304df729b  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/openresolv --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/openresolv; B=build/$QNX_ARCH/openresolv; mkdir -p $B
```

```bash
do_build() { rm -rf "$B" && cp -a "$S" "$B" && cd "$B" &&
  ./configure --os=qnx --prefix="$PREFIX" --sysconfdir="$PREFIX/etc" --sbindir="$PREFIX/sbin" \
    --libexecdir="$PREFIX/libexec/resolvconf" --rundir=/var/run --mandir="$PREFIX/share/man" && make; }
```

```bash
do_install() { make -C "$B" DESTDIR="$STAGE" install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- openresolv: resolvconf, the framework that merges the DNS information of several sources (DHCP clients, VPNs, static configuration) into resolv.conf, and optionally into local resolver configuration (unbound, dnsmasq, named, pdnsd). Shell scripts, the same for every architecture.
- Its own configure (not autoconf). --os=qnx: it would otherwise take the build machine's uname (Linux) and expect sha256sum/gpg2 there. resolvconf.conf lives in the package tree's own etc (hazard 124); the file written stays the system's /etc/resolv.conf, which QNX's resolver reads (resolv_conf= in resolvconf.conf changes it). The interface state goes under /var/run/resolvconf.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH openresolv`.

### 5. What it installs

14 files under `/usr/local`, 1 programs.

```
/usr/local/sbin/resolvconf
```

The complete list is `stage/<arch>/.manifest/openresolv` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa openresolv
ls dists/8.0/*/ | grep '^openresolv-'
```

This writes `openresolv-<ver>-r0.tar.gz` (run time) and `openresolv-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add openresolv
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/openresolv-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/openresolv-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_openresolv`](../../tests/src/) links openresolv the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-openresolv`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-openresolv
/usr/local/bin/qnx-port-tests/test_openresolv
```

```
== openresolv ==
  ok   ...
PASS openresolv
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_openresolv` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_openresolv`](../../tests/src/) -- a program linked against openresolv the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> openresolv`.

## Versions

This recipe builds **3.17.4**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add openresolv=<version>`), sources in `ported/openresolv/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates openresolv`; port one with `scripts/port-update openresolv`.

<details><summary>recipe.sh</summary>

```bash
# openresolv: resolvconf, the framework that merges the DNS information of
# several sources (DHCP clients, VPNs, static configuration) into
# resolv.conf, and optionally into local resolver configuration (unbound,
# dnsmasq, named, pdnsd). Shell scripts, the same for every architecture.
VERSION=3.17.4
TARBALL=https://github.com/NetworkConfiguration/openresolv/releases/download/v3.17.4/openresolv-3.17.4.tar.xz
SHA256=901d83a15520b80e117aeca8cb6f5b70ceb205ba050a638d45cebcf304df729b
BUILDSYS=custom
# Its own configure (not autoconf). --os=qnx: it would otherwise take the
# build machine's uname (Linux) and expect sha256sum/gpg2 there.
# resolvconf.conf lives in the package tree's own etc (hazard 124); the
# file written stays the system's /etc/resolv.conf, which QNX's resolver
# reads (resolv_conf= in resolvconf.conf changes it). The interface state
# goes under /var/run/resolvconf.
do_build() { rm -rf "$B" && cp -a "$S" "$B" && cd "$B" &&
  ./configure --os=qnx --prefix="$PREFIX" --sysconfdir="$PREFIX/etc" --sbindir="$PREFIX/sbin" \
    --libexecdir="$PREFIX/libexec/resolvconf" --rundir=/var/run --mandir="$PREFIX/share/man" && make; }
do_install() { make -C "$B" DESTDIR="$STAGE" install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
