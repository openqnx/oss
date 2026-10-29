# json-glib 1.10.6

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.gnome.org/GNOME/json-glib.git (tag `1.10.6`) |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`glib`](../glib/) |
| Run-time dependencies | [`gettext`](../gettext/) [`glib`](../glib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_json_glib`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install json-glib
oss-get install json-glib-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild json-glib` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (9 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib
```

<details><summary>Full build chain (9)</summary>

[`bzip2`](../bzip2/) [`gettext`](../gettext/) [`glib`](../glib/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`pcre2`](../pcre2/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 1.10.6 --recurse-submodules --shallow-submodules \
    https://gitlab.gnome.org/GNOME/json-glib.git src/json-glib
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/json-glib src/json-glib \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dintrospection=disabled \
      -Dgtk_doc=disabled \
      -Dtests=false \
      -Dinstalled_tests=false
ninja -C build/$QNX_ARCH/json-glib
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/json-glib install
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH json-glib`.

### 5. What it installs

82 files under `/usr/local`: 3 shared-library entries, 2 programs, 13 headers.

```
/usr/local/lib/libjson-glib-1.0.so
/usr/local/lib/libjson-glib-1.0.so.0
/usr/local/lib/libjson-glib-1.0.so.0.1000.6
/usr/local/bin/json-glib-format
/usr/local/bin/json-glib-validate
```

The complete list is `stage/<arch>/.manifest/json-glib` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH json-glib
ls repo/8.0/*/ | grep '^json-glib-'
```

This writes `json-glib-<ver>-r0.tar.gz` (run time) and `json-glib-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install json-glib
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/json-glib-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/json-glib-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_json_glib`](../../tests/src/) links json-glib the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_json_glib root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_json_glib'
```

```
== json-glib ==
  ok   ...
PASS json-glib
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.10.6; GIT=https://gitlab.gnome.org/GNOME/json-glib.git; REF=1.10.6
DEPS="glib"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dintrospection=disabled -Dgtk_doc=disabled -Dtests=false
                   -Dinstalled_tests=false )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
