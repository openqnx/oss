# docbook-xsl 1.79.2

> DocBook XSL stylesheets for converting DocBook XML to HTML and man pages

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/docbook/xslt10-stylesheets/releases/download/release%2F1.79.2/docbook-xsl-1.79.2.tar.bz2 |
| SHA-256 | `316524ea444e53208a2fb90eeb676af755da96e1417835ba5f5eb719c81fa371` |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_docbook_xsl`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add docbook-xsl
apk add docbook-xsl-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-docbook-xsl && /usr/local/bin/qnx-port-tests/test_docbook_xsl    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install docbook-xsl`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild docbook-xsl` performs exactly the same steps (2–5) in one command.

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
URL=https://github.com/docbook/xslt10-stylesheets/releases/download/release%2F1.79.2/docbook-xsl-1.79.2.tar.bz2
mkdir -p dist src/docbook-xsl
curl -fL -o dist/$(basename $URL) $URL
echo "316524ea444e53208a2fb90eeb676af755da96e1417835ba5f5eb719c81fa371  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/docbook-xsl --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/docbook-xsl; B=build/$QNX_ARCH/docbook-xsl; mkdir -p $B
```

```bash
do_configure() { :; }
```

```bash
do_build() { :; }
```

```bash
do_install() {
  local d="$STAGE$PREFIX/share/xml/docbook/xsl-stylesheets-$VERSION"; mkdir -p "$d"
  local s; s=$(ls -d "$S"/docbook-xsl-* | head -n 1)
  ( cd "$s" && cp -a VERSION VERSION.xsl catalog.xml common html manpages fo lib highlighting params xhtml xhtml-1_1 xhtml5 "$d/" )
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- docbook-xsl: the DocBook XSL stylesheets (DocBook XML to HTML, man pages, FO), as xsltproc applies them -- asciidoc's and many projects' man pages. Data only, under share/xml/docbook/xsl-stylesheets-<version>.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH docbook-xsl`.

### 5. What it installs

1174 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/docbook-xsl` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa docbook-xsl
ls dists/8.0/*/ | grep '^docbook-xsl-'
```

This writes `docbook-xsl-<ver>-r0.tar.gz` (run time) and `docbook-xsl-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add docbook-xsl
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/docbook-xsl-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/docbook-xsl-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_docbook_xsl`](../../tests/src/) links docbook-xsl the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-docbook-xsl`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-docbook-xsl
/usr/local/bin/qnx-port-tests/test_docbook_xsl
```

```
== docbook-xsl ==
  ok   ...
PASS docbook-xsl
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_docbook_xsl` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_docbook_xsl`](../../tests/src/) -- a program linked against docbook-xsl the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - work directory
   - manpages/docbook.xsl installed
   - stylesheet parsed (with its includes)
   - DocBook source parsed
   - xsltApplyStylesheet
   - qnxdemo.1 written
   - .TH QNXDEMO \"1\"
   - NAME: qnxdemo \\- demonstrate the stylesheets

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | docbook-xsl 1.79.2-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/docbook-xsl-1.79.2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | docbook-xsl 1.79.2-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/docbook-xsl-1.79.2-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.79.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add docbook-xsl=<version>`), sources in `ported/docbook-xsl/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates docbook-xsl`; port one with `scripts/port-update docbook-xsl`.

<details><summary>recipe.sh</summary>

```bash
# docbook-xsl: the DocBook XSL stylesheets (DocBook XML to HTML, man pages,
# FO), as xsltproc applies them -- asciidoc's and many projects' man pages.
# Data only, under share/xml/docbook/xsl-stylesheets-<version>.
VERSION=1.79.2
TARBALL=https://github.com/docbook/xslt10-stylesheets/releases/download/release%2F1.79.2/docbook-xsl-1.79.2.tar.bz2
FETCH_NAME=docbook-xsl-1.79.2.tar.bz2
SHA256=316524ea444e53208a2fb90eeb676af755da96e1417835ba5f5eb719c81fa371
DEPS=""
RUNTIME_DEPS="libxslt"
BUILDSYS=custom
source "$WS/scripts/lib/fonts.sh"
do_fetch() { qnx_data_fetch; }
do_configure() { :; }
do_build() { :; }
do_install() {
  local d="$STAGE$PREFIX/share/xml/docbook/xsl-stylesheets-$VERSION"; mkdir -p "$d"
  local s; s=$(ls -d "$S"/docbook-xsl-* | head -n 1)
  ( cd "$s" && cp -a VERSION VERSION.xsl catalog.xml common html manpages fo lib highlighting params xhtml xhtml-1_1 xhtml5 "$d/" )
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
