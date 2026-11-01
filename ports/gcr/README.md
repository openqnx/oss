# gcr 3.41.2

> GNOME library for displaying certificates and accessing key stores

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.gnome.org/GNOME/gcr.git (tag `3.41.2`) |
| Build system | Meson |
| QNX patches | 1 |
| Build dependencies | [`glib`](../glib/) [`libgcrypt`](../libgcrypt/) [`p11-kit`](../p11-kit/) [`libsecret`](../libsecret/) [`gtk3`](../gtk3/) |
| Run-time dependencies | [`cairo`](../cairo/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`gtk3`](../gtk3/) [`libgcrypt`](../libgcrypt/) [`p11-kit`](../p11-kit/) [`pango`](../pango/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_gcr`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `gcr` (programs, shared libraries, run-time data) and `gcr-dev` (headers, pkg-config/CMake files, static libraries; needs `gcr`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`gcr`](https://openqnx.github.io/oss/package/x86_64/gcr.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/gcr.html)) · [`gcr-dev`](https://openqnx.github.io/oss/package/x86_64/gcr-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/gcr-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add gcr` (dependencies come along); one version: `apk add gcr=<version>` | `apk del gcr` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install gcr` (dependencies first) | `oss-get remove gcr` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`gcr-dev` the same way, by its own name (`apk add gcr-dev`, `oss-get install gcr-dev`, by hand with its own file and list `gcr-dev.list`); remove it before `gcr`, which it needs.

```sh
# on the QNX target, as root
apk add gcr gcr-dev
apk add qnx-test-gcr && /usr/local/bin/qnx-port-tests/test_gcr    # its on-target test, from tests-8.0-<arch>
apk del gcr gcr-dev qnx-test-gcr
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/gcr.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/gcr.list
rm var/lib/manual/gcr.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild gcr` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (63 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib libgcrypt p11-kit libsecret gtk3
```

<details><summary>Full build chain (63)</summary>

[`at-spi2-core`](../at-spi2-core/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dbus`](../dbus/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gdbm`](../gdbm/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libepoxy`](../libepoxy/) [`libffi`](../libffi/) [`libgcrypt`](../libgcrypt/) [`libgpg-error`](../libgpg-error/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libsecret`](../libsecret/) [`libtasn1`](../libtasn1/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfixes`](../libxfixes/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxtst`](../libxtst/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`p11-kit`](../p11-kit/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 3.41.2 --recurse-submodules --shallow-submodules \
    https://gitlab.gnome.org/GNOME/gcr.git src/gcr
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/gcr < ports/gcr/patches/0001-qnx-xopen-source-value.patch
```

**[`0001-qnx-xopen-source-value.patch`](patches/0001-qnx-xopen-source-value.patch)** — gcr: give _XOPEN_SOURCE a value.

> QNX's <sys/platform.h> accepts _XOPEN_SOURCE only as 500, 600 or 700; a bare
> -D_XOPEN_SOURCE stops with "This version of XOPEN_SOURCE is not supported".
> 700 is the level that declares strptime(), which the flag is there for.

Touches: `gck/meson.build`, `gcr/meson.build` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/gcr src/gcr \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dgtk_doc=false \
      -Dintrospection=false \
      -Dgtk=true \
      -Dssh_agent=false \
      -Dsystemd=disabled
ninja -C build/$QNX_ARCH/gcr
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/gcr install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- GTK enabled: gcr-3 (the UI part, prompts and certificate viewer) is what GNOME Web (epiphany) needs; gcr-base-3 alone is not enough.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH gcr`.

### 5. What it installs

199 files under `/usr/local`: 9 shared-library entries, 1 programs, 60 headers.

```
/usr/local/lib/libgck-1.so
/usr/local/lib/libgck-1.so.0
/usr/local/lib/libgck-1.so.0.0.0
/usr/local/lib/libgcr-base-3.so
/usr/local/lib/libgcr-base-3.so.1
/usr/local/lib/libgcr-base-3.so.1.0.0
/usr/local/lib/libgcr-ui-3.so
/usr/local/lib/libgcr-ui-3.so.1
/usr/local/lib/libgcr-ui-3.so.1.0.0
/usr/local/bin/gcr-viewer
```

The complete list is `stage/<arch>/.manifest/gcr` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa gcr
ls dists/8.0/*/ | grep '^gcr-'
```

This writes `gcr-<ver>-r0.tar.gz` (run time) and `gcr-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add gcr
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/gcr-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/gcr-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_gcr`](../../tests/src/) links gcr the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-gcr`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-gcr
/usr/local/bin/qnx-port-tests/test_gcr
```

```
== gcr ==
  ok   ...
PASS gcr
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_gcr` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_gcr`](../../tests/src/) -- a program linked against gcr the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - gcr_parser_new
   - gcr_parser_parse_data on a PEM certificate + key bundle
   - two objects parsed
   - one is a Certificate
   - one is a Private Key
   - certificate label is its subject CN
   - PEM body decodes to the 891-byte DER certificate
   - gcr_simple_certificate_new
   - subject CN
   - issuer CN (self-signed)
   - subject O
   - subject OU
   - subject C
   - certificate is its own issuer
   - serial number 1234ABCD
   - RSA key size 2048
   - SHA-1 fingerprint matches openssl
   - SHA-256 fingerprint matches openssl
   - expiry year 2046 (20-year certificate)
   - gcr-viewer --version: GCR Certificate and Key Viewer -- 3.41.2

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | gcr 3.41.2-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/gcr-3.41.2-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | gcr 3.41.2-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/gcr-3.41.2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | gcr 3.41.2-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/gcr-3.41.2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | gcr 3.41.2-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/gcr-3.41.2-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **3.41.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add gcr=<version>`), sources in `ported/gcr/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates gcr`; port one with `scripts/port-update gcr`.

<details><summary>recipe.sh</summary>

```bash
VERSION=3.41.2; GIT=https://gitlab.gnome.org/GNOME/gcr.git; REF=3.41.2
DEPS="glib libgcrypt p11-kit libsecret gtk3"
BUILDSYS=meson
# GTK enabled: gcr-3 (the UI part, prompts and certificate viewer) is what
# GNOME Web (epiphany) needs; gcr-base-3 alone is not enough.
MESON_ARGS_EXTRA=( -Dgtk_doc=false -Dintrospection=false
                   -Dgtk=true -Dssh_agent=false -Dsystemd=disabled )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
