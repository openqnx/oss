# gnome-keyring 51.1

> GNOME Keyring: the session secret store daemon (org.freedesktop.secrets)

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://download.gnome.org/sources/gnome-keyring/51/gnome-keyring-51.1.tar.xz |
| SHA-256 | `dfe7adc5876ef0670e7eba2907799bdbb8875ce58bb4208e1a52169bd16a47ca` |
| Build system | Meson |
| QNX patches | 2 |
| Build dependencies | [`glib`](../glib/) [`gcr`](../gcr/) [`libgcrypt`](../libgcrypt/) [`libgpg-error`](../libgpg-error/) [`p11-kit`](../p11-kit/) |
| Run-time dependencies | [`gcr`](../gcr/) [`gettext`](../gettext/) [`glib`](../glib/) [`libgcrypt`](../libgcrypt/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_gnome_keyring`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `gnome-keyring`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`gnome-keyring`](https://openqnx.github.io/oss/package/x86_64/gnome-keyring.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/gnome-keyring.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add gnome-keyring` (dependencies come along); one version: `apk add gnome-keyring=<version>` | `apk del gnome-keyring` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install gnome-keyring` (dependencies first) | `oss-get remove gnome-keyring` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-gnome-keyring && /usr/local/bin/qnx-port-tests/test_gnome_keyring    # its on-target test, from tests-8.0-<arch>
apk del gnome-keyring qnx-test-gnome-keyring
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/gnome-keyring.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/gnome-keyring.list
rm var/lib/manual/gnome-keyring.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild gnome-keyring` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (64 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib gcr libgcrypt libgpg-error p11-kit
```

<details><summary>Full build chain (64)</summary>

[`at-spi2-core`](../at-spi2-core/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dbus`](../dbus/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gcr`](../gcr/) [`gdbm`](../gdbm/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libepoxy`](../libepoxy/) [`libffi`](../libffi/) [`libgcrypt`](../libgcrypt/) [`libgpg-error`](../libgpg-error/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libsecret`](../libsecret/) [`libtasn1`](../libtasn1/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfixes`](../libxfixes/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxtst`](../libxtst/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`p11-kit`](../p11-kit/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://download.gnome.org/sources/gnome-keyring/51/gnome-keyring-51.1.tar.xz
mkdir -p dist src/gnome-keyring
curl -fL -o dist/$(basename $URL) $URL
echo "dfe7adc5876ef0670e7eba2907799bdbb8875ce58bb4208e1a52169bd16a47ca  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/gnome-keyring --strip-components=1
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/gnome-keyring < ports/gnome-keyring/patches/0001-define-have-cmsgcred.patch
patch -p1 -d src/gnome-keyring < ports/gnome-keyring/patches/0002-qnx-send-scm-creds.patch
```

**[`0001-define-have-cmsgcred.patch`](patches/0001-define-have-cmsgcred.patch)** — Define HAVE_CMSGCRED from meson's struct cmsgcred probe.

> meson.build probes for struct cmsgcred (have_struct_cmsgcred) but never
> puts the answer into config.h, so egg-unix-credentials.c never sees
> HAVE_CMSGCRED. On QNX -- whose socket layer is FreeBSD's: struct cmsgcred,
> SCM_CREDS, LOCAL_CREDS, no SO_PEERCRED -- the code then takes the
> LOCAL_CREDS branch, which does not even compile ("'fd' undeclared"). Define
> HAVE_CMSGCRED from the probe, as the autotools build did: the control
> socket's peer is then identified by the kernel-filled cmsgcred that the
> client sends with SCM_CREDS (the mechanism libdbus uses on QNX, hazard 134).

Touches: `meson.build` · Upstream status: QNX-specific

**[`0002-qnx-send-scm-creds.patch`](patches/0002-qnx-send-scm-creds.patch)** — Send SCM_CREDS on QNX as on FreeBSD.

> egg_unix_credentials_write() sends its credentials byte with an SCM_CREDS
> control message only where LOCAL_CREDS is absent or on FreeBSD; elsewhere
> it writes the bare byte. QNX defines LOCAL_CREDS (<sys/un.h>) but its
> socket layer is FreeBSD's: the reader (built with HAVE_CMSGCRED) waits for
> the struct cmsgcred the kernel fills into an SCM_CREDS message, and every
> client was refused with "message from recvmsg() was not SCM_CREDS" --
> "gnome-keyring-daemon --start" could not reach a running daemon over its
> control socket. Send SCM_CREDS on QNX as on FreeBSD (libdbus and, with
> ports/glib's patch, GDBus do the same there, hazard 134).

Touches: `egg/egg-unix-credentials.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/gnome-keyring src/gnome-keyring \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dsystemd=disabled \
      -Dpam=false \
      -Dssh-agent=false \
      -Dselinux=disabled \
      -Dmanpage=false \
      -Ddebug-mode=false
ninja -C build/$QNX_ARCH/gnome-keyring
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/gnome-keyring install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- GNOME Keyring: gnome-keyring-daemon, the session's secret store (the org.freedesktop.secrets D-Bus service that libsecret clients use) with its PKCS#11 modules, and the gnome-keyring control tool. No systemd socket activation, no PAM helper (pam_gnome_keyring needs a desktop login), no SSH agent (gcr's ssh-agent replaces it upstream).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH gnome-keyring`.

### 5. What it installs

111 files under `/usr/local`, 3 programs.

```
/usr/local/bin/gnome-keyring
/usr/local/bin/gnome-keyring-3
/usr/local/bin/gnome-keyring-daemon
```

The complete list is `stage/<arch>/.manifest/gnome-keyring` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa gnome-keyring
ls dists/8.0/*/ | grep '^gnome-keyring-'
```

This writes `gnome-keyring-<ver>-r0.tar.gz` (run time) and `gnome-keyring-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add gnome-keyring
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/gnome-keyring-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/gnome-keyring-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_gnome_keyring`](../../tests/src/) links gnome-keyring the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-gnome-keyring`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-gnome-keyring
/usr/local/bin/qnx-port-tests/test_gnome_keyring
```

```
== gnome-keyring ==
  ok   ...
PASS gnome-keyring
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_gnome_keyring` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_gnome_keyring`](../../tests/src/) -- a program linked against gnome-keyring the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> gnome-keyring`.

## Versions

This recipe builds **51.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add gnome-keyring=<version>`), sources in `ported/gnome-keyring/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates gnome-keyring`; port one with `scripts/port-update gnome-keyring`.

<details><summary>recipe.sh</summary>

```bash
# GNOME Keyring: gnome-keyring-daemon, the session's secret store (the
# org.freedesktop.secrets D-Bus service that libsecret clients use) with
# its PKCS#11 modules, and the gnome-keyring control tool. No systemd
# socket activation, no PAM helper (pam_gnome_keyring needs a desktop
# login), no SSH agent (gcr's ssh-agent replaces it upstream).
VERSION=51.1
TARBALL=https://download.gnome.org/sources/gnome-keyring/51/gnome-keyring-51.1.tar.xz
SHA256=dfe7adc5876ef0670e7eba2907799bdbb8875ce58bb4208e1a52169bd16a47ca
DEPS="glib gcr libgcrypt libgpg-error p11-kit"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dsystemd=disabled -Dpam=false -Dssh-agent=false -Dselinux=disabled -Dmanpage=false -Ddebug-mode=false )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
