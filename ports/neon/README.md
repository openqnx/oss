# neon 0.37.1

> HTTP and WebDAV client library, with a C interface

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://notroj.github.io/neon/neon-0.37.1.tar.gz |
| SHA-256 | `a99b7262525a454d1065cf76dd17240fd808dfc4ef15636990ff83a5d0d9e740` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`openssl`](../openssl/) [`expat`](../expat/) [`zlib`](../zlib/) |
| Run-time dependencies | [`expat`](../expat/) [`openssl`](../openssl/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_neon`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `neon` (programs, shared libraries, run-time data) and `neon-dev` (headers, pkg-config/CMake files, static libraries; needs `neon`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`neon`](https://openqnx.github.io/oss/package/x86_64/neon.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/neon.html)) · [`neon-dev`](https://openqnx.github.io/oss/package/x86_64/neon-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/neon-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add neon` (dependencies come along); one version: `apk add neon=<version>` | `apk del neon` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install neon` (dependencies first) | `oss-get remove neon` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`neon-dev` the same way, by its own name (`apk add neon-dev`, `oss-get install neon-dev`, by hand with its own file and list `neon-dev.list`); remove it before `neon`, which it needs.

```sh
# on the QNX target, as root
apk add neon neon-dev
apk add qnx-test-neon && /usr/local/bin/qnx-port-tests/test_neon    # its on-target test, from tests-8.0-<arch>-m-z
apk del neon neon-dev qnx-test-neon
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/neon.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/neon.list
rm var/lib/manual/neon.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild neon` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (3 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild openssl expat zlib
```

### 2. Get the upstream source

```bash
URL=https://notroj.github.io/neon/neon-0.37.1.tar.gz
mkdir -p dist src/neon
curl -fL -o dist/$(basename $URL) $URL
echo "a99b7262525a454d1065cf76dd17240fd808dfc4ef15636990ff83a5d0d9e740  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/neon --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/neon; B=build/$QNX_ARCH/neon; mkdir -p $B
```

```bash
do_install() { make -C "$B" DESTDIR="$STAGE" install-lib install-headers install-config; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- neon: HTTP and WebDAV client library (sessions, request/response, auth, PROPFIND/PROPPATCH, locking), with SSL/TLS through OpenSSL, XML through expat and gzip content decoding through zlib. The release tarball: it ships the generated configure (the git tree needs autogen.sh and the docs toolchain).
- The tarball carries no generated man pages (doc/man is the XML source, built with xmlto), and `make install` stops on the missing doc/man/*.3: install the library, headers and neon-config/neon.pc only.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH neon`.

### 5. What it installs

29 files under `/usr/local`: 3 shared-library entries, 1 programs, 24 headers.

```
/usr/local/lib/libneon.so
/usr/local/lib/libneon.so.27
/usr/local/lib/libneon.so.27.10.1
/usr/local/bin/neon-config
```

The complete list is `stage/<arch>/.manifest/neon` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa neon
ls dists/8.0/*/ | grep '^neon-'
```

This writes `neon-<ver>-r0.tar.gz` (run time) and `neon-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add neon
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/neon-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/neon-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_neon`](../../tests/src/) links neon the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-neon`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-neon
/usr/local/bin/qnx-port-tests/test_neon
```

```
== neon ==
  ok   ...
PASS neon
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_neon` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_neon`](../../tests/src/) -- a program linked against neon the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - test server listening on 127.0.0.1
   - ne_sock_init
   - built with SSL and zlib support
   - GET /hello: 200 OK
   - GET /hello: body
   - response header Content-Type
   - two requests on one persistent connection
   - gzip response decoded (zlib)
   - ne_putbuf: 201 Created
   - server received the exact bytes
   - GET returns what was PUT
   - ne_delete
   - ne_mkcol: 201 Created
   - dispatch of a missing resource: status 404
   - ne_delete on 404: NE_ERROR naming the status
   - Basic auth after a 401 challenge
   - credentials asked for once, for realm qnxtest
   - PROPFIND: 207 with two resources
   - PROPFIND: getcontentlength and displayname values
   - ne_uri_parse
   - ne_path_unescape
   - ne_base64
   - ne_rfc1123_parse
   - ne_rfc1123_date

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | neon 0.37.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/neon-0.37.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | neon 0.37.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/neon-0.37.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.37.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add neon=<version>`), sources in `ported/neon/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates neon`; port one with `scripts/port-update neon`.

<details><summary>recipe.sh</summary>

```bash
# neon: HTTP and WebDAV client library (sessions, request/response, auth,
# PROPFIND/PROPPATCH, locking), with SSL/TLS through OpenSSL, XML through
# expat and gzip content decoding through zlib.
# The release tarball: it ships the generated configure (the git tree needs
# autogen.sh and the docs toolchain).
VERSION=0.37.1
TARBALL=https://notroj.github.io/neon/neon-0.37.1.tar.gz
SHA256=a99b7262525a454d1065cf76dd17240fd808dfc4ef15636990ff83a5d0d9e740
BUILDSYS=autotools
DEPS="openssl expat zlib"
CONF_ARGS_EXTRA=( --enable-shared --disable-static --with-ssl=openssl --with-expat
                  --without-libproxy --without-gssapi --disable-nls --disable-debug )
# The tarball carries no generated man pages (doc/man is the XML source,
# built with xmlto), and `make install` stops on the missing doc/man/*.3:
# install the library, headers and neon-config/neon.pc only.
do_install() { make -C "$B" DESTDIR="$STAGE" install-lib install-headers install-config; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
