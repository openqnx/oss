# enchant 2.8.2

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/AbiWord/enchant/releases/download/v2.8.2/enchant-2.8.2.tar.gz |
| SHA-256 | `8f19535adb5577b83b00e02f330fe9b9eb40dd21f19e2899636fc4d3a7696375` |
| Build system | GNU Autotools |
| QNX patches | 1 |
| Build dependencies | [`glib`](../glib/) [`aspell`](../aspell/) |
| Run-time dependencies | [`aspell`](../aspell/) [`gettext`](../gettext/) [`glib`](../glib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_enchant`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install enchant
oss-get install enchant-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild enchant` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (10 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib aspell
```

<details><summary>Full build chain (10)</summary>

[`aspell`](../aspell/) [`bzip2`](../bzip2/) [`gettext`](../gettext/) [`glib`](../glib/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`pcre2`](../pcre2/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
URL=https://github.com/AbiWord/enchant/releases/download/v2.8.2/enchant-2.8.2.tar.gz
mkdir -p dist src/enchant
curl -fL -o dist/$(basename $URL) $URL
echo "8f19535adb5577b83b00e02f330fe9b9eb40dd21f19e2899636fc4d3a7696375  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/enchant --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/enchant < ports/enchant/patches/0001-qnx-find-shared-library-fullname-via-dladdr.patch
```

**[`0001-qnx-find-shared-library-fullname-via-dladdr.patch`](patches/0001-qnx-find-shared-library-fullname-via-dladdr.patch)** — Teach gnulib's relocatable.c how to find a shared library on QNX.

> find_shared_library_fullname() is implemented only for Linux and Cygwin,
> both by scanning /proc/self/maps.  QNX has no such file in that format, so
> on QNX the function compiles to nothing, get_shared_library_fullname()
> always returns NULL, and relocate() hands every path straight back.
>
> That failure is silent and total: enchant dlopens its spell-checking
> providers from a directory it composes from the configure-time PKGLIBDIR,
> so with --enable-relocatable in effect but relocation not working, a
> broker installed anywhere other than the configured prefix constructs
> successfully and reports no providers at all.
>
> dladdr() answers exactly the question the /proc scan is asking -- which
> object contains this address -- and QNX 8.0 has it in libc, with Dl_info
> exposed under __EXT_UNIX_MISC (which _QNX_SOURCE turns on).

Touches: `libgnu/relocatable.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/enchant && cd build/$QNX_ARCH/enchant
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/enchant/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static \
    --enable-relocatable \
    --with-aspell \
    --without-hunspell \
    --without-nuspell \
    --without-voikko \
    --without-hspell \
    --without-applespell
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- The release tarball, not the git tag.  Enchant's git tree carries gnulib and gl-mod/bootstrap as submodules and expects ./bootstrap to run gnulib-tool; the tarball ships a generated configure and the gnulib sources it uses, so no submodule clone (and no gnulib-tool here) is needed.  README hazard 74.
- --enable-relocatable: enchant dlopens its providers from a path it composes from the compiled-in PKGLIBDIR ("/usr/local/lib/enchant-2").  Without relocation that path is absolute, so the library finds no provider at all unless the tree is installed at exactly the configured prefix -- and the on-target test bundle is deliberately relocatable.  gnulib's relocate() then derives the directory from where the library actually is.
- lib/Makefile.am forces config.h in with GCC's long-form "--include config.h". qcc parses its own command line first and rejects the long form outright ("cc: unknown option: '--include'"); -include is the universally accepted spelling and is exactly equivalent.  Applied to Makefile.in too so the tarball builds without re-running automake. Two build-machine fixes, both in lib/Makefile.{am,in} (patched in the .in too so the release tarball builds without re-running automake): 1. AM_CPPFLAGS forces config.h in with GCC's long-form "--include config.h". qcc parses its own command line before handing the rest to gcc and rejects the long form outright ("cc: unknown option: '--include'").  -include is the universally accepted spelling and is exactly equivalent. 2. nodist_doc_DATA runs "groff -mandoc -Thtml" to render the man page.  The html device needs groff's full install, which the build machine need not have, and the output is documentation for the build host -- nothing the target uses.  The man page itself is still installed.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH enchant`.

### 5. What it installs

12 files under `/usr/local`: 2 shared-library entries, 2 programs, 2 headers.

```
/usr/local/lib/libenchant-2.so
/usr/local/lib/libenchant-2.so.10
/usr/local/bin/enchant-2
/usr/local/bin/enchant-lsmod-2
```

The complete list is `stage/<arch>/.manifest/enchant` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH enchant
ls repo/8.0/*/ | grep '^enchant-'
```

This writes `enchant-<ver>-r0.tar.gz` (run time) and `enchant-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install enchant
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/enchant-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/enchant-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_enchant`](../../tests/src/) links enchant the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_enchant root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_enchant'
```

```
== enchant ==
  ok   ...
PASS enchant
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.8.2
# The release tarball, not the git tag.  Enchant's git tree carries gnulib and
# gl-mod/bootstrap as submodules and expects ./bootstrap to run gnulib-tool;
# the tarball ships a generated configure and the gnulib sources it uses, so
# no submodule clone (and no gnulib-tool here) is needed.  README hazard 74.
TARBALL=https://github.com/AbiWord/enchant/releases/download/v2.8.2/enchant-2.8.2.tar.gz
SHA256=8f19535adb5577b83b00e02f330fe9b9eb40dd21f19e2899636fc4d3a7696375
DEPS="glib aspell"
BUILDSYS=autotools
# --enable-relocatable: enchant dlopens its providers from a path it composes
# from the compiled-in PKGLIBDIR ("/usr/local/lib/enchant-2").  Without
# relocation that path is absolute, so the library finds no provider at all
# unless the tree is installed at exactly the configured prefix -- and the
# on-target test bundle is deliberately relocatable.  gnulib's relocate() then
# derives the directory from where the library actually is.
CONF_ARGS_EXTRA=( --enable-shared --disable-static --enable-relocatable
                  --with-aspell --without-hunspell
                  --without-nuspell --without-voikko --without-hspell --without-applespell )

# lib/Makefile.am forces config.h in with GCC's long-form "--include config.h".
# qcc parses its own command line first and rejects the long form outright
# ("cc: unknown option: '--include'"); -include is the universally accepted
# spelling and is exactly equivalent.  Applied to Makefile.in too so the
# tarball builds without re-running automake.
# Two build-machine fixes, both in lib/Makefile.{am,in} (patched in the .in too
# so the release tarball builds without re-running automake):
#
#  1. AM_CPPFLAGS forces config.h in with GCC's long-form "--include config.h".
#     qcc parses its own command line before handing the rest to gcc and rejects
#     the long form outright ("cc: unknown option: '--include'").  -include is
#     the universally accepted spelling and is exactly equivalent.
#  2. nodist_doc_DATA runs "groff -mandoc -Thtml" to render the man page.  The
#     html device needs groff's full install, which the build machine need not
#     have, and the output is documentation for the build host -- nothing the
#     target uses.  The man page itself is still installed.
do_patch() {
  cd "$SRCROOT" || return 1
  # 1. AM_CPPFLAGS forces config.h in with GCC's long-form "--include config.h".
  #    qcc parses its own command line before handing the rest to gcc and
  #    rejects the long form outright ("cc: unknown option: '--include'").
  #    -include is the universally accepted spelling and is equivalent.
  # 2. nodist_doc_DATA renders the man pages to HTML with "groff -Thtml".  The
  #    html device is not in groff-base, it is documentation for the *build*
  #    machine, and nothing on the target reads it.  The man pages themselves
  #    are still installed.  Patched in Makefile.in as well as Makefile.am so
  #    the release tarball builds without re-running automake.
  sed -i -e 's/--include config\.h/-include config.h/' \
         -e 's/^nodist_doc_DATA = .*/nodist_doc_DATA =/' \
         lib/Makefile.am lib/Makefile.in src/Makefile.am src/Makefile.in
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
