# ca-certificates 20261020

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://curl.se/ca/cacert-2026-10-20.pem |
| SHA-256 | `796f1389bfea13c0ed78f663b765baaff54a508e5483786bd6ece73c5e3e3cbe` |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add ca-certificates
apk add ca-certificates-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install ca-certificates`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild ca-certificates` performs exactly the same steps (2–5) in one command.

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
URL=https://curl.se/ca/cacert-2026-10-20.pem
mkdir -p dist src/ca-certificates
curl -fL -o dist/$(basename $URL) $URL
echo "796f1389bfea13c0ed78f663b765baaff54a508e5483786bd6ece73c5e3e3cbe  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/ca-certificates --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/ca-certificates; B=build/$QNX_ARCH/ca-certificates; mkdir -p $B
```

```bash
do_build() { :; }
```

```bash
do_install() { install -d "$STAGE$PREFIX/ssl/certs" "$STAGE$PREFIX/etc/ssl/certs" && \
  install -m644 "$S/cacert.pem" "$STAGE$PREFIX/ssl/cert.pem" && \
  ln -sfn ../cert.pem "$STAGE$PREFIX/ssl/certs/ca-certificates.crt" && \
  ln -sfn ../../../ssl/cert.pem "$STAGE$PREFIX/etc/ssl/certs/ca-certificates.crt"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- The Mozilla CA certificate bundle (as extracted and published by the curl project), so TLS clients on the target can verify servers. Installs ssl/cert.pem -- the default CA file of this tree's OpenSSL, which is built with OPENSSLDIR=/usr/local/ssl -- and the same file as etc/ssl/certs/ca-certificates.crt, where other software looks.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH ca-certificates`.

### 5. What it installs

3 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/ca-certificates` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa ca-certificates
ls dists/8.0/*/ | grep '^ca-certificates-'
```

This writes `ca-certificates-<ver>-r0.tar.gz` (run time) and `ca-certificates-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add ca-certificates
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/ca-certificates-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/ca-certificates-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | ca-certificates 20261020-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/ca-certificates-20261020-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | ca-certificates 20261020-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/ca-certificates-20261020-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **20261020**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add ca-certificates=<version>`), sources in `ported/ca-certificates/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates ca-certificates`; port one with `scripts/port-update ca-certificates`.

<details><summary>recipe.sh</summary>

```bash
# The Mozilla CA certificate bundle (as extracted and published by the curl
# project), so TLS clients on the target can verify servers. Installs
# ssl/cert.pem -- the default CA file of this tree's OpenSSL, which is built
# with OPENSSLDIR=/usr/local/ssl -- and the same file as
# etc/ssl/certs/ca-certificates.crt, where other software looks.
VERSION=20261020
TARBALL=https://curl.se/ca/cacert-2026-10-20.pem
SHA256=796f1389bfea13c0ed78f663b765baaff54a508e5483786bd6ece73c5e3e3cbe
BUILDSYS=custom
do_fetch() { mkdir -p "$SRCROOT" "$WS/dist" && local f="$WS/dist/$(basename "$TARBALL")"; [ -f "$f" ] || curl -fsSL -o "$f" "$TARBALL"; \
  echo "$SHA256  $f" | sha256sum -c --status && cp "$f" "$SRCROOT/cacert.pem"; }
do_build() { :; }
do_install() { install -d "$STAGE$PREFIX/ssl/certs" "$STAGE$PREFIX/etc/ssl/certs" && \
  install -m644 "$S/cacert.pem" "$STAGE$PREFIX/ssl/cert.pem" && \
  ln -sfn ../cert.pem "$STAGE$PREFIX/ssl/certs/ca-certificates.crt" && \
  ln -sfn ../../../ssl/cert.pem "$STAGE$PREFIX/etc/ssl/certs/ca-certificates.crt"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
