# dnssec-root 20241105

> DNSSEC root zone trust anchors (the root KSKs) from IANA

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://data.iana.org/root-anchors/root-anchors.xml |
| SHA-256 | `3ccaab38830025ee0a0f6c1f25769427544f81ea2865aa860468f3ef5278b908` |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_dnssec_root`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `dnssec-root`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`dnssec-root`](https://openqnx.github.io/oss/package/x86_64/dnssec-root.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/dnssec-root.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add dnssec-root` (dependencies come along); one version: `apk add dnssec-root=<version>` | `apk del dnssec-root` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install dnssec-root` (dependencies first) | `oss-get remove dnssec-root` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-dnssec-root && /usr/local/bin/qnx-port-tests/test_dnssec_root    # its on-target test, from tests-8.0-<arch>
apk del dnssec-root qnx-test-dnssec-root
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/dnssec-root.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/dnssec-root.list
rm var/lib/manual/dnssec-root.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild dnssec-root` performs exactly the same steps (2–5) in one command.

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
URL=https://data.iana.org/root-anchors/root-anchors.xml
mkdir -p dist src/dnssec-root
curl -fL -o dist/$(basename $URL) $URL
echo "3ccaab38830025ee0a0f6c1f25769427544f81ea2865aa860468f3ef5278b908  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/dnssec-root --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/dnssec-root; B=build/$QNX_ARCH/dnssec-root; mkdir -p $B
```

```bash
do_build() {
  mkdir -p "$B" && python3 -I - "$S/root-anchors.xml" "$B/trusted-key.key" <<'PY'
import sys, xml.etree.ElementTree as ET
t = ET.parse(sys.argv[1]).getroot()
lines = []
for kd in t.findall("KeyDigest"):
    pk = kd.findtext("PublicKey")
    if kd.get("validUntil") or not pk:
        continue
    lines.append(". IN DNSKEY %s 3 %s %s" % (kd.findtext("Flags"), kd.findtext("Algorithm"), pk.strip()))
if not lines:
    sys.exit("no current trust anchor with a public key")
open(sys.argv[2], "w").write("\n".join(sorted(lines, key=lambda l: l.split()[6])) + "\n")
PY
}
```

```bash
do_install() { install -Dm644 "$B/trusted-key.key" "$STAGE$PREFIX/share/dnssec-root/trusted-key.key" &&
  install -Dm644 "$S/root-anchors.xml" "$STAGE$PREFIX/share/dnssec-root/root-anchors.xml"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- The DNSSEC root zone trust anchors, from IANA's root-anchors.xml: the root KSKs (20326, and 38696 of the 2024 rollover) as DNSKEY records in share/dnssec-root/trusted-key.key -- the file validating resolvers (unbound's trust-anchor-file, dnsmasq, ldns tools) are pointed at -- and the XML itself next to it (unbound-anchor's format). Data, the same for every architecture. VERSION is the date IANA published this content (Last-Modified of root-anchors.xml); the download is pinned by SHA-256.
- trusted-key.key: one ". IN DNSKEY <flags> 3 <alg> <key>" line per anchor that is valid now (has no validUntil) and carries its public key, as the Alpine/QNX packages generate it with IANA's get_trust_anchor.py.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH dnssec-root`.

### 5. What it installs

2 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/dnssec-root` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa dnssec-root
ls dists/8.0/*/ | grep '^dnssec-root-'
```

This writes `dnssec-root-<ver>-r0.tar.gz` (run time) and `dnssec-root-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add dnssec-root
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/dnssec-root-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/dnssec-root-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_dnssec_root`](../../tests/src/) links dnssec-root the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-dnssec-root`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-dnssec-root
/usr/local/bin/qnx-port-tests/test_dnssec_root
```

```
== dnssec-root ==
  ok   ...
PASS dnssec-root
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_dnssec_root` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_dnssec_root`](../../tests/src/) -- a program linked against dnssec-root the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - trusted-key.key is installed
   - two DNSKEY records for \".\": flags 257 (KSK), protocol 3, algorithm 8
   - KSK-2017: key tag 20326, DS digest E06D44B8...EC8D
   - KSK-2024: key tag 38696, DS digest 683D2D0A...2B16
   - root-anchors.xml lists the same DS digests

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | dnssec-root 20241105-r0 | ✅ PASS | 2026-11-10 | [log](../../tests/logs/aarch64le/dnssec-root-20241105-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | dnssec-root 20241105-r0 | ✅ PASS | 2026-11-04 | [log](../../tests/logs/x86_64/dnssec-root-20241105-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **20241105**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add dnssec-root=<version>`), sources in `ported/dnssec-root/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates dnssec-root`; port one with `scripts/port-update dnssec-root`.

<details><summary>recipe.sh</summary>

```bash
# The DNSSEC root zone trust anchors, from IANA's root-anchors.xml: the
# root KSKs (20326, and 38696 of the 2024 rollover) as DNSKEY records in
# share/dnssec-root/trusted-key.key -- the file validating resolvers
# (unbound's trust-anchor-file, dnsmasq, ldns tools) are pointed at -- and
# the XML itself next to it (unbound-anchor's format). Data, the same for
# every architecture. VERSION is the date IANA published this content
# (Last-Modified of root-anchors.xml); the download is pinned by SHA-256.
VERSION=20241105
TARBALL=https://data.iana.org/root-anchors/root-anchors.xml
SHA256=3ccaab38830025ee0a0f6c1f25769427544f81ea2865aa860468f3ef5278b908
BUILDSYS=custom
do_fetch() { mkdir -p "$SRCROOT" "$WS/dist" && local f="$WS/dist/dnssec-root-anchors-$VERSION.xml"; [ -f "$f" ] || curl -fsSL -o "$f" "$TARBALL"; \
  echo "$SHA256  $f" | sha256sum -c --status && cp "$f" "$SRCROOT/root-anchors.xml"; }
# trusted-key.key: one ". IN DNSKEY <flags> 3 <alg> <key>" line per anchor
# that is valid now (has no validUntil) and carries its public key, as the
# Alpine/QNX packages generate it with IANA's get_trust_anchor.py.
do_build() {
  mkdir -p "$B" && python3 -I - "$S/root-anchors.xml" "$B/trusted-key.key" <<'PY'
import sys, xml.etree.ElementTree as ET
t = ET.parse(sys.argv[1]).getroot()
lines = []
for kd in t.findall("KeyDigest"):
    pk = kd.findtext("PublicKey")
    if kd.get("validUntil") or not pk:
        continue
    lines.append(". IN DNSKEY %s 3 %s %s" % (kd.findtext("Flags"), kd.findtext("Algorithm"), pk.strip()))
if not lines:
    sys.exit("no current trust anchor with a public key")
open(sys.argv[2], "w").write("\n".join(sorted(lines, key=lambda l: l.split()[6])) + "\n")
PY
}
do_install() { install -Dm644 "$B/trusted-key.key" "$STAGE$PREFIX/share/dnssec-root/trusted-key.key" &&
  install -Dm644 "$S/root-anchors.xml" "$STAGE$PREFIX/share/dnssec-root/root-anchors.xml"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
