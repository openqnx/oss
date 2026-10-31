# libvncserver 0.9.15

> C libraries for implementing VNC servers and clients

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/LibVNC/libvncserver.git (tag `LibVNCServer-0.9.15`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`zlib`](../zlib/) [`lzo`](../lzo/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`gnutls`](../gnutls/) [`openssl`](../openssl/) [`sdl2`](../sdl2/) |
| Run-time dependencies | [`gnutls`](../gnutls/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`lzo`](../lzo/) [`openssl`](../openssl/) [`sdl2`](../sdl2/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libvncserver`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add libvncserver
apk add libvncserver-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-libvncserver && /usr/local/bin/qnx-port-tests/test_libvncserver    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install libvncserver`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libvncserver` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (23 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild zlib lzo libjpeg-turbo libpng gnutls openssl sdl2
```

<details><summary>Full build chain (23)</summary>

[`brotli`](../brotli/) [`expat`](../expat/) [`gettext`](../gettext/) [`gmp`](../gmp/) [`gnutls`](../gnutls/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libidn2`](../libidn2/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libtasn1`](../libtasn1/) [`libunistring`](../libunistring/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`nettle`](../nettle/) [`openssl`](../openssl/) [`sdl2`](../sdl2/) [`wayland`](../wayland/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b LibVNCServer-0.9.15 --recurse-submodules --shallow-submodules \
    https://github.com/LibVNC/libvncserver.git src/libvncserver
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/libvncserver; B=build/$QNX_ARCH/libvncserver; mkdir -p $B
```

```bash
do_install() {
  DESTDIR="$STAGE" cmake --install "$B" || return 1
  install -Dm755 "$B/examples/client/SDLvncviewer" "$STAGE$PREFIX/bin/SDLvncviewer"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- libvncserver: LibVNCServer and LibVNCClient, the VNC (RFB) protocol as C libraries, with the Tight/JPEG, ZRLE and other encodings TurboVNC and TigerVNC servers use -- and SDLvncviewer, a VNC viewer on SDL, which on QNX draws straight onto QNX Screen. With VirtualGL on a Linux machine (vglrun in a TurboVNC session) this shows OpenGL programs such as RViz on QNX.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libvncserver`.

### 5. What it installs

20 files under `/usr/local`: 6 shared-library entries, 1 programs, 7 headers.

```
/usr/local/lib/libvncclient.so
/usr/local/lib/libvncclient.so.0.9.15
/usr/local/lib/libvncclient.so.1
/usr/local/lib/libvncserver.so
/usr/local/lib/libvncserver.so.0.9.15
/usr/local/lib/libvncserver.so.1
/usr/local/bin/SDLvncviewer
```

The complete list is `stage/<arch>/.manifest/libvncserver` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libvncserver
ls dists/8.0/*/ | grep '^libvncserver-'
```

This writes `libvncserver-<ver>-r0.tar.gz` (run time) and `libvncserver-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libvncserver
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libvncserver-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libvncserver-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libvncserver`](../../tests/src/) links libvncserver the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libvncserver`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libvncserver
/usr/local/bin/qnx-port-tests/test_libvncserver
```

```
== libvncserver ==
  ok   ...
PASS libvncserver
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libvncserver` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libvncserver`](../../tests/src/) -- a program linked against libvncserver the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - rfbGetScreen
   - server listening on 127.0.0.1
   - raw: the image, exactly
   - Tight, lossless: the image, exactly
   - Tight+JPEG (TurboVNC's encoding): within JPEG error
   - ZRLE: the image, exactly

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libvncserver 0.9.15-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/libvncserver-0.9.15-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libvncserver 0.9.15-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/libvncserver-0.9.15-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.9.15**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libvncserver=<version>`), sources in `ported/libvncserver/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libvncserver`; port one with `scripts/port-update libvncserver`.

<details><summary>recipe.sh</summary>

```bash
# libvncserver: LibVNCServer and LibVNCClient, the VNC (RFB) protocol as C
# libraries, with the Tight/JPEG, ZRLE and other encodings TurboVNC and
# TigerVNC servers use -- and SDLvncviewer, a VNC viewer on SDL, which on QNX
# draws straight onto QNX Screen. With VirtualGL on a Linux machine (vglrun
# in a TurboVNC session) this shows OpenGL programs such as RViz on QNX.
VERSION=0.9.15; GIT=https://github.com/LibVNC/libvncserver.git; REF=LibVNCServer-0.9.15
DEPS="zlib lzo libjpeg-turbo libpng gnutls openssl sdl2"
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=( -DWITH_SDL=ON -DWITH_GTK=OFF -DWITH_FFMPEG=OFF -DWITH_SYSTEMD=OFF -DWITH_LIBSSHTUNNEL=OFF
                   -DWITH_GCRYPT=OFF -DWITH_EXAMPLES=ON -DWITH_TESTS=OFF )
do_install() {
  DESTDIR="$STAGE" cmake --install "$B" || return 1
  install -Dm755 "$B/examples/client/SDLvncviewer" "$STAGE$PREFIX/bin/SDLvncviewer"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
