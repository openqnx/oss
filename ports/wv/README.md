# wv 1.2.9

> Library and converters for Microsoft Word 97-2003 (.doc) documents

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://deb.debian.org/debian/pool/main/w/wv/wv_1.2.9.orig.tar.gz |
| SHA-256 | `4c730d3b325c0785450dd3a043eeb53e1518598c4f41f155558385dd2635c19d` |
| Build system | GNU Autotools |
| QNX patches | 2 |
| Build dependencies | [`glib`](../glib/) [`libgsf`](../libgsf/) [`zlib`](../zlib/) [`libpng`](../libpng/) |
| Run-time dependencies | [`gettext`](../gettext/) [`glib`](../glib/) [`libgsf`](../libgsf/) [`libpng`](../libpng/) [`libxml2`](../libxml2/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_wv`](../../tests/src/) |

## Install and remove on a target

Not published as a package yet: build it as below, then `scripts/qnxpkg` makes `wv` and `wv-dev`, which install and remove like every other package ([INSTALL.md](../../INSTALL.md), [docs/packages.md](../../docs/packages.md)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild wv` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (20 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib libgsf zlib libpng
```

<details><summary>Full build chain (20)</summary>

[`bzip2`](../bzip2/) [`expat`](../expat/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`glib`](../glib/) [`libffi`](../libffi/) [`libgsf`](../libgsf/) [`libiconv`](../libiconv/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`libxml2`](../libxml2/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`pcre2`](../pcre2/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://deb.debian.org/debian/pool/main/w/wv/wv_1.2.9.orig.tar.gz
mkdir -p dist src/wv
curl -fL -o dist/$(basename $URL) $URL
echo "4c730d3b325c0785450dd3a043eeb53e1518598c4f41f155558385dd2635c19d  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/wv --strip-components=1
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/wv < ports/wv/patches/0001-qnx-no-ansi-flag.patch
patch -p1 -d src/wv < ports/wv/patches/0002-qnx-missing-declarations.patch
```

**[`0001-qnx-no-ansi-flag.patch`](patches/0001-qnx-no-ansi-flag.patch)** — wv: -std=gnu89 in place of -ansi.

> configure gives GCC "-ansi" (ANSI_CFLAGS, added to every compile). QNX's qcc
> driver rejects that option ("cc: -ansi no longer supported"), so nothing
> compiled. -std=gnu89 is the language level -ansi selected for this 1990s C
> code, with the GNU extensions the system headers expect. configure is
> changed along with configure.ac, so the tarball builds without autoreconf.

Touches: `configure`, `configure.ac` · Upstream status: QNX-specific

**[`0002-qnx-missing-declarations.patch`](patches/0002-qnx-missing-declarations.patch)** — wv: declare what wvSummary and wvparse.c call (pointers cut to 32 bits).

> wvSummary.c calls gsf_doc_meta_data_new(), gsf_doc_meta_data_foreach(),
> gsf_doc_prop_get_val() and gsf_doc_prop_get_link() without including
> <gsf/gsf-doc-meta-data.h>. Built as C89 (configure's -ansi, -std=gnu89 here)
> the compiler takes them as functions returning int, without a warning, and
> the returned pointers lose their upper 32 bits: on QNX, where heap addresses
> are above 4 GB, wvSummary crashed in libgsf (gsf_doc_meta_data_insert) on
> most documents. The header is included.
>
> wvparse.c calls wvOLEDecode_gsf() (laolareplace.c) with no prototype in
> scope; it returns int, so it worked, but the prototype is added to wv.h so
> that the arguments are checked too.

Touches: `wv.h`, `wvSummary.c` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/wv && cd build/$QNX_ARCH/wv
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/wv/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static \
    --without-libwmf
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- wv: the library that reads Microsoft Word 2000/97/95/6 documents (the .doc format), with converters (wvHtml, wvText, wvRTF, ...). AbiWord's .doc import is built on it.
- abisource.com, the upstream site, no longer serves it; Debian keeps the unchanged release tarball.
- patches/0001 changes configure.ac and configure: keep the generated files newer than their sources, or make reruns autoconf/automake.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH wv`.

### 5. What it installs

482 files under `/usr/local`: 2 shared-library entries, 16 programs, 1 headers.

```
/usr/local/lib/libwv-1.2.so.4
/usr/local/lib/libwv.so
/usr/local/bin/wvAbw
/usr/local/bin/wvCleanLatex
/usr/local/bin/wvConvert
/usr/local/bin/wvDVI
/usr/local/bin/wvDocBook
/usr/local/bin/wvHtml
/usr/local/bin/wvLatex
/usr/local/bin/wvMime
/usr/local/bin/wvPDF
/usr/local/bin/wvPS
/usr/local/bin/wvRTF
/usr/local/bin/wvSummary
/usr/local/bin/wvText
/usr/local/bin/wvVersion
...
```

The complete list is `stage/<arch>/.manifest/wv` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa wv
ls dists/8.0/*/ | grep '^wv-'
```

This writes `wv-<ver>-r0.tar.gz` (run time) and `wv-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add wv
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/wv-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/wv-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_wv`](../../tests/src/) links wv the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-wv`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-wv
/usr/local/bin/qnx-port-tests/test_wv
```

```
== wv ==
  ok   ...
PASS wv
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_wv` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_wv`](../../tests/src/) -- a program linked against wv the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> wv`.

## Versions

This recipe builds **1.2.9**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add wv=<version>`), sources in `ported/wv/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates wv`; port one with `scripts/port-update wv`.

<details><summary>recipe.sh</summary>

```bash
# wv: the library that reads Microsoft Word 2000/97/95/6 documents (the .doc
# format), with converters (wvHtml, wvText, wvRTF, ...). AbiWord's .doc import
# is built on it.
VERSION=1.2.9
# abisource.com, the upstream site, no longer serves it; Debian keeps the
# unchanged release tarball.
TARBALL=https://deb.debian.org/debian/pool/main/w/wv/wv_1.2.9.orig.tar.gz
SHA256=4c730d3b325c0785450dd3a043eeb53e1518598c4f41f155558385dd2635c19d
DEPS="glib libgsf zlib libpng"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --without-libwmf )
# patches/0001 changes configure.ac and configure: keep the generated files
# newer than their sources, or make reruns autoconf/automake.
do_patch() {
  cd "$SRCROOT" && touch aclocal.m4 && sleep 1 && touch configure config.h.in && \
  find . -name 'GNUmakefile.in' -exec touch {} +
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
