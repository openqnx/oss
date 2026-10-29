# gstreamer 1.24.10

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.freedesktop.org/gstreamer/gstreamer.git (tag `1.24.10`) |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`glib`](../glib/) [`libxml2`](../libxml2/) |
| Run-time dependencies | [`gettext`](../gettext/) [`glib`](../glib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_gstreamer`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install gstreamer
oss-get install gstreamer-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild gstreamer` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild glib libxml2
```

<details><summary>Full build chain (11)</summary>

[`bzip2`](../bzip2/) [`gettext`](../gettext/) [`glib`](../glib/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`libxml2`](../libxml2/) [`ncurses`](../ncurses/) [`pcre2`](../pcre2/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 1.24.10 --recurse-submodules --shallow-submodules \
    https://gitlab.freedesktop.org/gstreamer/gstreamer.git src/gstreamer
```

The project lives in the `subprojects/gstreamer/` subdirectory of the repository.

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/gstreamer src/gstreamer/subprojects/gstreamer \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dtests=disabled \
      -Dexamples=disabled \
      -Dbenchmarks=disabled \
      -Dtools=enabled \
      -Dgst_debug=true \
      -Dintrospection=disabled \
      -Ddoc=disabled \
      -Dptp-helper-permissions=none \
      # \
      setuid \
      helper \
      install \
      needs \
      root \
      -Dlibunwind=disabled \
      -Dlibdw=disabled \
      -Ddbghelp=disabled \
      -Dbash-completion=disabled
ninja -C build/$QNX_ARCH/gstreamer
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/gstreamer install
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH gstreamer`.

### 5. What it installs

212 files under `/usr/local`: 15 shared-library entries, 4 programs, 123 headers.

```
/usr/local/lib/libgstbase-1.0.so
/usr/local/lib/libgstbase-1.0.so.0
/usr/local/lib/libgstbase-1.0.so.0.2410.0
/usr/local/lib/libgstcheck-1.0.so
/usr/local/lib/libgstcheck-1.0.so.0
/usr/local/lib/libgstcheck-1.0.so.0.2410.0
/usr/local/lib/libgstcontroller-1.0.so
/usr/local/lib/libgstcontroller-1.0.so.0
/usr/local/lib/libgstcontroller-1.0.so.0.2410.0
/usr/local/lib/libgstnet-1.0.so
/usr/local/lib/libgstnet-1.0.so.0
/usr/local/lib/libgstnet-1.0.so.0.2410.0
/usr/local/lib/libgstreamer-1.0.so
/usr/local/lib/libgstreamer-1.0.so.0
/usr/local/lib/libgstreamer-1.0.so.0.2410.0
/usr/local/bin/gst-inspect-1.0
...
```

The complete list is `stage/<arch>/.manifest/gstreamer` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH gstreamer
ls repo/8.0/*/ | grep '^gstreamer-'
```

This writes `gstreamer-<ver>-r0.tar.gz` (run time) and `gstreamer-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install gstreamer
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/gstreamer-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/gstreamer-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_gstreamer`](../../tests/src/) links gstreamer the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_gstreamer root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_gstreamer'
```

```
== gstreamer ==
  ok   ...
PASS gstreamer
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.24.10; GIT=https://gitlab.freedesktop.org/gstreamer/gstreamer.git; REF=1.24.10
DEPS="glib libxml2"
SUBDIR=subprojects/gstreamer
BUILDSYS=meson
MESON_ARGS_EXTRA=(
  -Dtests=disabled -Dexamples=disabled -Dbenchmarks=disabled
  -Dtools=enabled -Dgst_debug=true
  -Dintrospection=disabled -Ddoc=disabled
  -Dptp-helper-permissions=none     # setuid helper install needs root
  -Dlibunwind=disabled -Dlibdw=disabled -Ddbghelp=disabled
  -Dbash-completion=disabled
)
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
