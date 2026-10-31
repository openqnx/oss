# libsecret 0.21.4

> GObject library for storing and retrieving secrets via Secret Service

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.gnome.org/GNOME/libsecret.git (tag `0.21.4`) |
| Build system | Meson |
| QNX patches | 1 |
| Build dependencies | [`glib`](../glib/) [`libgcrypt`](../libgcrypt/) |
| Run-time dependencies | [`glib`](../glib/) [`libgcrypt`](../libgcrypt/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libsecret`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `libsecret` (programs, shared libraries, run-time data) and `libsecret-dev` (headers, pkg-config/CMake files, static libraries; needs `libsecret`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`libsecret`](https://openqnx.github.io/oss/package/x86_64/libsecret.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libsecret.html)) · [`libsecret-dev`](https://openqnx.github.io/oss/package/x86_64/libsecret-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libsecret-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add libsecret` (dependencies come along); one version: `apk add libsecret=<version>` | `apk del libsecret` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install libsecret` (dependencies first) | `oss-get remove libsecret` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`libsecret-dev` the same way, by its own name (`apk add libsecret-dev`, `oss-get install libsecret-dev`, by hand with its own file and list `libsecret-dev.list`); remove it before `libsecret`, which it needs.

```sh
# on the QNX target, as root
apk add libsecret libsecret-dev
apk add qnx-test-libsecret && /usr/local/bin/qnx-port-tests/test_libsecret    # its on-target test
apk del libsecret libsecret-dev qnx-test-libsecret
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/libsecret.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/libsecret.list
rm var/lib/manual/libsecret.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libsecret` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (11 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib libgcrypt
```

<details><summary>Full build chain (11)</summary>

[`bzip2`](../bzip2/) [`gettext`](../gettext/) [`glib`](../glib/) [`libffi`](../libffi/) [`libgcrypt`](../libgcrypt/) [`libgpg-error`](../libgpg-error/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`pcre2`](../pcre2/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 0.21.4 --recurse-submodules --shallow-submodules \
    https://gitlab.gnome.org/GNOME/libsecret.git src/libsecret
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/libsecret < ports/libsecret/patches/0001-fix-undeclared-fd-in-local-creds-path.patch
```

**[`0001-fix-undeclared-fd-in-local-creds-path.patch`](patches/0001-fix-undeclared-fd-in-local-creds-path.patch)** — libsecret: fix an undeclared variable in the LOCAL_CREDS path.

> The error message uses "fd" where the parameter is "sock". The branch is
> compiled only where LOCAL_CREDS exists and CMSGCRED does not -- QNX, not
> Linux or the BSDs -- so the typo has gone unnoticed upstream.

Touches: `egg/egg-unix-credentials.c` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/libsecret src/libsecret \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dintrospection=false \
      -Dgtk_doc=false \
      -Dmanpage=false \
      -Dvapi=false \
      -Dcrypto=libgcrypt \
      -Dtpm2=false \
      -Dbash_completion=disabled
ninja -C build/$QNX_ARCH/libsecret
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/libsecret install
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libsecret`.

### 5. What it installs

83 files under `/usr/local`: 3 shared-library entries, 1 programs, 16 headers.

```
/usr/local/lib/libsecret-1.so
/usr/local/lib/libsecret-1.so.0
/usr/local/lib/libsecret-1.so.0.0.0
/usr/local/bin/secret-tool
```

The complete list is `stage/<arch>/.manifest/libsecret` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libsecret
ls dists/8.0/*/ | grep '^libsecret-'
```

This writes `libsecret-<ver>-r0.tar.gz` (run time) and `libsecret-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libsecret
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libsecret-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libsecret-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libsecret`](../../tests/src/) links libsecret the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libsecret`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libsecret
/usr/local/bin/qnx-port-tests/test_libsecret
```

```
== libsecret ==
  ok   ...
PASS libsecret
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libsecret` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libsecret`](../../tests/src/) -- a program linked against libsecret the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - schema is usable
   - secret_value_new
   - value round trip
   - content type preserved
   - secret_value_ref
   - got a service proxy
   - absence of the service is reported as an error

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libsecret 0.21.4-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/libsecret-0.21.4-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libsecret 0.21.4-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/libsecret-0.21.4-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.21.4**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libsecret=<version>`), sources in `ported/libsecret/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libsecret`; port one with `scripts/port-update libsecret`.

<details><summary>recipe.sh</summary>

```bash
VERSION=0.21.4; GIT=https://gitlab.gnome.org/GNOME/libsecret.git; REF=0.21.4
DEPS="glib libgcrypt"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dintrospection=false -Dgtk_doc=false -Dmanpage=false
                   -Dvapi=false -Dcrypto=libgcrypt -Dtpm2=false -Dbash_completion=disabled )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
