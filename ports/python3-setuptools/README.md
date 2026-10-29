# python3-setuptools 80.8.0

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://files.pythonhosted.org/packages/58/29/93c53c098d301132196c3238c312825324740851d77a8500a2462c0fd888/setuptools-80.8.0-py3-none-any.whl |
| SHA-256 | `95a60484590d24103af13b686121328cc2736bee85de8936383111e421b9edc0` |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | [`python3`](../python3/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install python3-setuptools
oss-get install python3-setuptools-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild python3-setuptools` performs exactly the same steps (2–5) in one command.

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
URL=https://files.pythonhosted.org/packages/58/29/93c53c098d301132196c3238c312825324740851d77a8500a2462c0fd888/setuptools-80.8.0-py3-none-any.whl
mkdir -p dist src/python3-setuptools
curl -fL -o dist/$(basename $URL) $URL
echo "95a60484590d24103af13b686121328cc2736bee85de8936383111e421b9edc0  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/python3-setuptools --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/python3-setuptools; B=build/$QNX_ARCH/python3-setuptools; mkdir -p $B
```

```bash
do_build() { :; }
```

```bash
do_install() { qnx_py_install_pure; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- setuptools (version as in the QNX Everywhere 8.0.4r1 image). Pure Python: the published wheel is platform independent, so it is installed as released (see scripts/lib/pywheel.sh).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH python3-setuptools`.

### 5. What it installs

512 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/python3-setuptools` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH python3-setuptools
ls repo/8.0/*/ | grep '^python3-setuptools-'
```

This writes `python3-setuptools-<ver>-r0.tar.gz` (run time) and `python3-setuptools-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install python3-setuptools
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/python3-setuptools-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/python3-setuptools-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
# setuptools (version as in the QNX Everywhere 8.0.4r1 image).
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=80.8.0
TARBALL=https://files.pythonhosted.org/packages/58/29/93c53c098d301132196c3238c312825324740851d77a8500a2462c0fd888/setuptools-80.8.0-py3-none-any.whl
SHA256=95a60484590d24103af13b686121328cc2736bee85de8936383111e421b9edc0
DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
