# yt-dlp 2026.10.26

> Download or stream video and audio from YouTube and other sites

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://files.pythonhosted.org/packages/7f/3d/16ea7d5638cf585991f2c5705e8083688dde633d32d0871f2404e0f4d827/yt_dlp-2026.10.26-py3-none-any.whl |
| SHA-256 | `a05c5b5d8562f7e8a62f5334348b406af4930d56d3a65f8021d14af0e22339ae` |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | [`python3`](../python3/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_yt_dlp`](../../tests/src/) |

## Install and remove on a target

Not published as a package yet: build it as below, then `scripts/qnxpkg` makes `yt-dlp` and `yt-dlp-dev`, which install and remove like every other package ([INSTALL.md](../../INSTALL.md), [docs/packages.md](../../docs/packages.md)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild yt-dlp` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (15 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild python3
```

<details><summary>Full build chain (15)</summary>

[`bzip2`](../bzip2/) [`expat`](../expat/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://files.pythonhosted.org/packages/7f/3d/16ea7d5638cf585991f2c5705e8083688dde633d32d0871f2404e0f4d827/yt_dlp-2026.10.26-py3-none-any.whl
mkdir -p dist src/yt-dlp
curl -fL -o dist/$(basename $URL) $URL
echo "a05c5b5d8562f7e8a62f5334348b406af4930d56d3a65f8021d14af0e22339ae  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/yt-dlp --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/yt-dlp; B=build/$QNX_ARCH/yt-dlp; mkdir -p $B
```

```bash
do_build() { :; }
```

```bash
do_install() { qnx_py_install_pure && qnx_py_scripts "$SRCROOT"/*.whl; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- yt-dlp: download or stream video and audio from YouTube and many other sites (a command line program). Pure Python: the published wheel is platform independent, so it is installed as released (scripts/lib/pywheel.sh); its console script is written as a launcher (qnx_py_scripts). Without the optional extras it needs only Python; ffmpeg (ported) merges separate video and audio streams and converts formats when it is installed. It plays nothing itself: hand its stream URL to a player (parole, vlc). No DRM: sites protected by Widevine (Netflix and the like) are not supported.
- Python modules are imported, not linked: what they need is named here.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH yt-dlp`.

### 5. What it installs

1058 files under `/usr/local`, 1 programs.

```
/usr/local/bin/yt-dlp
```

The complete list is `stage/<arch>/.manifest/yt-dlp` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa yt-dlp
ls dists/8.0/*/ | grep '^yt-dlp-'
```

This writes `yt-dlp-<ver>-r0.tar.gz` (run time) and `yt-dlp-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add yt-dlp
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/yt-dlp-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/yt-dlp-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_yt_dlp`](../../tests/src/) links yt-dlp the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-yt-dlp`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-yt-dlp
/usr/local/bin/qnx-port-tests/test_yt_dlp
```

```
== yt-dlp ==
  ok   ...
PASS yt-dlp
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_yt_dlp` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_yt_dlp`](../../tests/src/) -- a program linked against yt-dlp the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> yt-dlp`.

## Versions

This recipe builds **2026.10.26**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add yt-dlp=<version>`), sources in `ported/yt-dlp/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates yt-dlp`; port one with `scripts/port-update yt-dlp`.

<details><summary>recipe.sh</summary>

```bash
# yt-dlp: download or stream video and audio from YouTube and many other
# sites (a command line program). Pure Python: the published wheel is
# platform independent, so it is installed as released (scripts/lib/pywheel.sh);
# its console script is written as a launcher (qnx_py_scripts). Without the
# optional extras it needs only Python; ffmpeg (ported) merges separate
# video and audio streams and converts formats when it is installed.
# It plays nothing itself: hand its stream URL to a player (parole, vlc).
# No DRM: sites protected by Widevine (Netflix and the like) are not supported.
VERSION=2026.10.26
TARBALL=https://files.pythonhosted.org/packages/7f/3d/16ea7d5638cf585991f2c5705e8083688dde633d32d0871f2404e0f4d827/yt_dlp-2026.10.26-py3-none-any.whl
SHA256=a05c5b5d8562f7e8a62f5334348b406af4930d56d3a65f8021d14af0e22339ae
DEPS="python3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure && qnx_py_scripts "$SRCROOT"/*.whl; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
