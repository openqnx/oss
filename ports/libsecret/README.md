# libsecret 0.21.4

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

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install libsecret
oss-get install libsecret-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

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
./scripts/qnxpkg -a $QNX_ARCH libsecret
ls repo/8.0/*/ | grep '^libsecret-'
```

This writes `libsecret-<ver>-r0.tar.gz` (run time) and `libsecret-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install libsecret
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/libsecret-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/libsecret-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_libsecret`](../../tests/src/) links libsecret the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libsecret root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_libsecret'
```

```
== libsecret ==
  ok   ...
PASS libsecret
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

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
