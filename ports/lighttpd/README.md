# lighttpd 1.4.77

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://git.lighttpd.net/lighttpd/lighttpd1.4.git (tag `lighttpd-1.4.77`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`openssl`](../openssl/) [`pcre2`](../pcre2/) [`zlib`](../zlib/) |
| Run-time dependencies | [`openssl`](../openssl/) [`pcre2`](../pcre2/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add lighttpd
apk add lighttpd-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install lighttpd`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild lighttpd` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (4 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild openssl pcre2 zlib
```

<details><summary>Full build chain (4)</summary>

[`bzip2`](../bzip2/) [`openssl`](../openssl/) [`pcre2`](../pcre2/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b lighttpd-1.4.77 --recurse-submodules --shallow-submodules \
    https://git.lighttpd.net/lighttpd/lighttpd1.4.git src/lighttpd
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/lighttpd -B build/$QNX_ARCH/lighttpd -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DWITH_OPENSSL=ON \
      -DWITH_PCRE2=ON \
      -DWITH_ZLIB=ON \
      -DWITH_BZIP=OFF \
      -DBUILD_STATIC=OFF
cmake --build build/$QNX_ARCH/lighttpd -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/lighttpd
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH lighttpd`.

### 5. What it installs

23 files under `/usr/local`, 2 programs.

```
/usr/local/sbin/lighttpd
/usr/local/sbin/lighttpd-angel
```

The complete list is `stage/<arch>/.manifest/lighttpd` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa lighttpd
ls dists/8.0/*/ | grep '^lighttpd-'
```

This writes `lighttpd-<ver>-r0.tar.gz` (run time) and `lighttpd-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add lighttpd
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/lighttpd-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/lighttpd-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | lighttpd 1.4.77-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | lighttpd 1.4.77-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **1.4.77**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add lighttpd=<version>`), sources in `ported/lighttpd/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates lighttpd`; port one with `scripts/port-update lighttpd`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.4.77; GIT=https://git.lighttpd.net/lighttpd/lighttpd1.4.git; REF=lighttpd-1.4.77
DEPS="openssl pcre2 zlib"
CMAKE_ARGS_EXTRA=( -DWITH_OPENSSL=ON -DWITH_PCRE2=ON -DWITH_ZLIB=ON -DWITH_BZIP=OFF -DBUILD_STATIC=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
