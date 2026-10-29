# libnotify 0.8.3

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.gnome.org/GNOME/libnotify.git (tag `0.8.3`) |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`glib`](../glib/) [`gdk-pixbuf`](../gdk-pixbuf/) |
| Run-time dependencies | [`gdk-pixbuf`](../gdk-pixbuf/) [`glib`](../glib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libnotify`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install libnotify
oss-get install libnotify-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libnotify` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (12 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib gdk-pixbuf
```

<details><summary>Full build chain (12)</summary>

[`bzip2`](../bzip2/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`pcre2`](../pcre2/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 0.8.3 --recurse-submodules --shallow-submodules \
    https://gitlab.gnome.org/GNOME/libnotify.git src/libnotify
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/libnotify src/libnotify \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dtests=false \
      -Dintrospection=disabled \
      -Dman=false \
      -Dgtk_doc=false \
      -Ddocbook_docs=disabled
ninja -C build/$QNX_ARCH/libnotify
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/libnotify install
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libnotify`.

### 5. What it installs

9 files under `/usr/local`: 3 shared-library entries, 1 programs, 4 headers.

```
/usr/local/lib/libnotify.so
/usr/local/lib/libnotify.so.4
/usr/local/lib/libnotify.so.4.0.0
/usr/local/bin/notify-send
```

The complete list is `stage/<arch>/.manifest/libnotify` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH libnotify
ls repo/8.0/*/ | grep '^libnotify-'
```

This writes `libnotify-<ver>-r0.tar.gz` (run time) and `libnotify-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install libnotify
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/libnotify-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/libnotify-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_libnotify`](../../tests/src/) links libnotify the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libnotify root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_libnotify'
```

```
== libnotify ==
  ok   ...
PASS libnotify
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=0.8.3; GIT=https://gitlab.gnome.org/GNOME/libnotify.git; REF=0.8.3
DEPS="glib gdk-pixbuf"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dtests=false -Dintrospection=disabled -Dman=false -Dgtk_doc=false
                   -Ddocbook_docs=disabled )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
