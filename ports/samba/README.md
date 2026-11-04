# samba 4.25.1

> SMB/CIFS file sharing: client tools, libsmbclient and the smbd/nmbd server

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://download.samba.org/pub/samba/stable/samba-4.25.1.tar.gz |
| SHA-256 | `7f367545bca69aa96897628f82f0f04d4b66b3bcb36cb978b9bb952c4b4c428d` |
| Build system | custom (see recipe) |
| QNX patches | 7 |
| Build dependencies | [`gnutls`](../gnutls/) [`tdb`](../tdb/) [`libiconv`](../libiconv/) [`icu`](../icu/) [`zlib`](../zlib/) [`libxcrypt`](../libxcrypt/) |
| Run-time dependencies | [`gnutls`](../gnutls/) [`icu`](../icu/) [`libiconv`](../libiconv/) [`libxcrypt`](../libxcrypt/) [`tdb`](../tdb/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_samba`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `samba` (programs, shared libraries, run-time data) and `samba-dev` (headers, pkg-config/CMake files, static libraries; needs `samba`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`samba`](https://openqnx.github.io/oss/package/x86_64/samba.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/samba.html)) · [`samba-dev`](https://openqnx.github.io/oss/package/x86_64/samba-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/samba-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add samba` (dependencies come along); one version: `apk add samba=<version>` | `apk del samba` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install samba` (dependencies first) | `oss-get remove samba` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`samba-dev` the same way, by its own name (`apk add samba-dev`, `oss-get install samba-dev`, by hand with its own file and list `samba-dev.list`); remove it before `samba`, which it needs.

```sh
# on the QNX target, as root
apk add samba samba-dev
apk add qnx-test-samba && /usr/local/bin/qnx-port-tests/test_samba    # its on-target test, from tests-8.0-<arch>-m-z
apk del samba samba-dev qnx-test-samba
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/samba.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/samba.list
rm var/lib/manual/samba.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild samba` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild gnutls tdb libiconv icu zlib libxcrypt
```

<details><summary>Full build chain (15)</summary>

[`brotli`](../brotli/) [`gettext`](../gettext/) [`gmp`](../gmp/) [`gnutls`](../gnutls/) [`icu`](../icu/) [`libiconv`](../libiconv/) [`libidn2`](../libidn2/) [`libtasn1`](../libtasn1/) [`libunistring`](../libunistring/) [`libxcrypt`](../libxcrypt/) [`ncurses`](../ncurses/) [`nettle`](../nettle/) [`tdb`](../tdb/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://download.samba.org/pub/samba/stable/samba-4.25.1.tar.gz
mkdir -p dist src/samba
curl -fL -o dist/$(basename $URL) $URL
echo "7f367545bca69aa96897628f82f0f04d4b66b3bcb36cb978b9bb952c4b4c428d  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/samba --strip-components=1
```

### 3. Apply the QNX patches

7 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/samba < ports/samba/patches/0001-native-heimdal-tools.patch
patch -p1 -d src/samba < ports/samba/patches/0002-heimdal-auxv-t.patch
patch -p1 -d src/samba < ports/samba/patches/0003-util-time-sys-time-h.patch
patch -p1 -d src/samba < ports/samba/patches/0004-qnx-atfork-child-mutexes.patch
patch -p1 -d src/samba < ports/samba/patches/0005-qnx-printf-null.patch
patch -p1 -d src/samba < ports/samba/patches/0006-qnx-dirfd-o-directory.patch
patch -p1 -d src/samba < ports/samba/patches/0007-qnx-fstatvfs-dirfd.patch
```

**[`0001-native-heimdal-tools.patch`](patches/0001-native-heimdal-tools.patch)** — Use native asn1_compile/compile_et in cross builds.

> Heimdal's asn1_compile and compile_et generate sources while Samba builds,
> so they must run on the build machine. waf's use_hostcc only adds
> -D_SAMBA_HOSTCC_ -- it still compiles them with the target compiler -- so
> a cross build produces QNX executables it then tries to run. With
> SAMBA_NATIVE_HEIMDAL_TOOLS naming a directory that holds native builds of
> both (the recipe builds them from the same source with a native
> configure), those are used and the target ones are not built, the way a
> system Heimdal's tools are used (USING_SYSTEM_ASN1_COMPILE/COMPILE_ET).

Touches: `third_party/heimdal_build/wscript_build` · Upstream status: QNX-specific

**[`0002-heimdal-auxv-t.patch`](patches/0002-heimdal-auxv-t.patch)** — Probe for auxv_t (QNX's <sys/auxv.h> has one).

> roken's getauxval.h defines its own auxv_t unless HAVE_AUXV_T, which
> Heimdal's autoconf build probes for and Samba's waf build never does. QNX's
> <sys/auxv.h> defines auxv_t (included when HAVE_SYS_AUXV_H), so roken's
> definition conflicts: "conflicting types for 'auxv_t'". Probe it.

Touches: `third_party/heimdal_build/wscript_configure` · Upstream status: QNX-specific

**[`0003-util-time-sys-time-h.patch`](patches/0003-util-time-sys-time-h.patch)** — Include <sys/time.h> for struct timeval.

> lib/util/time.h declares functions and structures using struct timeval
> after including only <time.h>. glibc's <time.h> happens to define it; POSIX
> puts it in <sys/time.h>, and QNX's <time.h> does not have it ("field
> 'start_time' has incomplete type" in auth/common_auth.h).

Touches: `lib/util/time.h` · Upstream status: QNX-specific

**[`0004-qnx-atfork-child-mutexes.patch`](patches/0004-qnx-atfork-child-mutexes.patch)** — Re-initialise atfork-locked mutexes in the child on QNX.

> tevent, pthreadpool and tfork lock their mutexes in a pthread_atfork()
> prepare handler and unlock them in the parent and child handlers. On QNX
> the child's thread does not own a mutex its parent locked:
> pthread_mutex_unlock() returns EPERM (hazard 127), and these handlers
> abort()/assert on that -- every fork of a threaded Samba process (smbd
> forks one child per client) would die. In the child, re-initialise the
> mutex instead, which leaves it unlocked as intended (the QNX ports' Samba
> 4.11 patch did the same).

Touches: `lib/pthreadpool/pthreadpool.c`, `lib/tevent/tevent.c`, `lib/util/tfork.c` · Upstream status: QNX-specific

**[`0005-qnx-printf-null.patch`](patches/0005-qnx-printf-null.patch)** — Print NULL %s arguments as (null) on QNX.

> QNX's printf family dereferences a NULL "%s" argument (hazard 99) where
> glibc prints "(null)", and Samba's debug and error messages print optional
> strings that way. QNX's libc has a hook for this: when the variable
> output_for_percent_s_NULL points to a string, that string is printed
> instead. libreplace, which every Samba program and library links, sets it
> from a constructor (the QNX ports' Samba 4.11 defined it in smbd and nmbd).

Touches: `lib/replace/replace.c` · Upstream status: QNX-specific

**[`0006-qnx-dirfd-o-directory.patch`](patches/0006-qnx-dirfd-o-directory.patch)** — Reopen directory handles with O_DIRECTORY on QNX.

> On QNX, openat()/unlinkat()/renameat() and the other *at() functions
> reject a directory file descriptor that was not opened with O_DIRECTORY
> (ENOTDIR, hazard 25). smbd opens "pathref" handles of path components with
> plain O_RDONLY (Linux uses O_PATH, which QNX lacks) and then uses them as
> the dirfd of later *at() calls: deleting a file through SMB failed with
> NT_STATUS_NOT_A_DIRECTORY ("unlink failed with error Not a directory").
> When a read-only open without O_DIRECTORY yields a directory, reopen it
> with O_DIRECTORY on QNX.

Touches: `source3/modules/vfs_default.c` · Upstream status: QNX-specific

**[`0007-qnx-fstatvfs-dirfd.patch`](patches/0007-qnx-fstatvfs-dirfd.patch)** — Retry fstatvfs() on a plain descriptor when QNX refuses an O_DIRECTORY one.

> QNX's fstatvfs() fails with ENOSYS on a descriptor opened with
> O_DIRECTORY (checked on the targets: O_RDONLY works, O_RDONLY|O_DIRECTORY
> does not), and smbd asks for a share's free space through the directory
> handle it opened that way: every smbclient "ls" ended with "Error in
> dskattr: NT_STATUS_NOT_SUPPORTED" (and exit status 1). On ENOSYS, retry on
> a plain O_RDONLY descriptor of the same directory.

Touches: `lib/util/statvfs.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/samba; B=build/$QNX_ARCH/samba; mkdir -p $B
```

```bash
do_build() {
  mkdir -p "$_SMB_NATIVE" && ( _smb_yapp ) && ( _smb_native ) || return 1
  rm -rf "$B" && cp -a "$S" "$B" && cd "$B" &&
  cp "$WS/ports/samba/cross-answers-$QNX_ARCH.txt" "$B/cross-answers.txt" &&
  _smb_env ./configure --prefix="$PREFIX" --enable-fhs --sysconfdir="$PREFIX/etc" --localstatedir="$PREFIX/var" \
    --cross-compile --cross-answers="$B/cross-answers.txt" --hostcc=gcc \
    --disable-python --without-ad-dc --without-ads --without-ldap --without-winbind --without-pam \
    --without-acl-support --without-quotas --without-systemd --without-libarchive --without-gpgme \
    --disable-cups --disable-iprint --disable-avahi --disable-glusterfs --disable-cephfs --disable-cephrgw \
    --without-fam --without-automount --without-dmapi --without-utmp --without-json --without-regedit \
    --without-winexe --disable-spotlight --without-ldb-lmdb --without-libunwind --without-lttng \
    --without-cluster-support --without-kernel-keyring --without-systemd-userdb --disable-rpath \
    --disable-rpath-install --bundled-libraries='!tdb,talloc,tevent,ldb,popt,cmocka' \
    --with-shared-modules='!vfs_snapper' --with-libiconv="$STAGE_SYSROOT$PREFIX" --without-gettext &&
  _smb_env make -j"$(nproc)"
}
```

```bash
do_install() { cd "$B" && _smb_env make install DESTDIR="$STAGE" || return 1
  local d; for d in log/samba lib/samba lib/samba/private lock/samba cache/samba run/samba; do
    mkdir -p "$STAGE$PREFIX/var/$d" && : > "$STAGE$PREFIX/var/$d/.keep" || return 1; done
  chmod 700 "$STAGE$PREFIX/var/lib/samba/private"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Samba: SMB/CIFS file sharing -- the client tools (smbclient, nmblookup, smbget, smbtree, rpcclient, net, ...) with libsmbclient, and the file server (smbd, nmbd) for a standalone server: shares, tdbsam users, SMB2/3 with signing and encryption (GnuTLS). Not built: the AD domain controller, AD membership (ads), winbind, LDAP, PAM, ACL/quota support, printing (CUPS) and the Python bindings -- each needs a Linux facility, a library not ported here or a desktop/directory service.
- The build machine needs what a native Samba configure needs (gcc, perl, python3, flex, bison, the GnuTLS development files): see _smb_native. pidl needs Parse::Yapp on the build machine; it is fetched from CPAN (pinned) and installed into the native helper directory.
- Heimdal's asn1_compile and compile_et run while Samba builds; waf builds them with the target compiler. A native configure of the same source builds just those two (patches/0001 makes the cross build use them via SAMBA_NATIVE_HEIMDAL_TOOLS).
- The cross build. Notes: - configure questions that need a test program run are answered from ports/samba/cross-answers-<arch>.txt, recorded on QNX targets (see the comments there); waf builds in the source tree, so in a copy. - staging headers as -isystem (through qcc's -Wc,): after Samba's and Heimdal's own include directories (libmd's staged md5.h/sha.h would shadow Heimdal's hcrypto ones), before the SDP's (its ICU 67 headers would shadow the staged ICU). - -lregex and -lfsnotify: regcomp() and inotify are not in QNX's libc (hazards 2, 64). - --enable-fhs: required for a /usr/local prefix; state under /usr/local/var, configuration in /usr/local/etc/samba. - tdb is the ported one (ports/tdb); talloc, tevent, ldb, popt and cmocka are Samba's bundled private copies (lib/samba, *-private-samba). - no gettext: libintl's printf replacements were not linked into Heimdal's com_err; messages stay English. - readline is not used: its 8.2 headers fail Samba's strict-prototypes probe, so smbclient reads commands without line editing.
- The state directories Samba creates only one level of (it does not mkdir -p): empty ones are not packaged, so each gets a placeholder and a packaged samba starts with its compiled-in paths.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH samba`.

### 5. What it installs

359 files under `/usr/local`: 41 shared-library entries, 43 programs, 74 headers.

```
/usr/local/lib/libdcerpc-binding.so
/usr/local/lib/libdcerpc-binding.so.0
/usr/local/lib/libdcerpc-binding.so.0.0.1
/usr/local/lib/libdcerpc-server-core.so
/usr/local/lib/libdcerpc-server-core.so.0
/usr/local/lib/libdcerpc-server-core.so.0.0.1
/usr/local/lib/libndr-krb5pac.so
/usr/local/lib/libndr-krb5pac.so.0
/usr/local/lib/libndr-krb5pac.so.0.0.1
/usr/local/lib/libndr-nbt.so
/usr/local/lib/libndr-nbt.so.0
/usr/local/lib/libndr-nbt.so.0.0.1
/usr/local/lib/libndr-standard.so
/usr/local/lib/libndr-standard.so.0
/usr/local/lib/libndr-standard.so.0.0.1
/usr/local/lib/libndr.so
...
```

The complete list is `stage/<arch>/.manifest/samba` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa samba
ls dists/8.0/*/ | grep '^samba-'
```

This writes `samba-<ver>-r0.tar.gz` (run time) and `samba-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add samba
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/samba-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/samba-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_samba`](../../tests/src/) links samba the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-samba`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-samba
/usr/local/bin/qnx-port-tests/test_samba
```

```
== samba ==
  ok   ...
PASS samba
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_samba` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_samba`](../../tests/src/) -- a program linked against samba the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> samba`.

## Versions

This recipe builds **4.25.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add samba=<version>`), sources in `ported/samba/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates samba`; port one with `scripts/port-update samba`.

<details><summary>recipe.sh</summary>

```bash
# Samba: SMB/CIFS file sharing -- the client tools (smbclient, nmblookup,
# smbget, smbtree, rpcclient, net, ...) with libsmbclient, and the file
# server (smbd, nmbd) for a standalone server: shares, tdbsam users,
# SMB2/3 with signing and encryption (GnuTLS). Not built: the AD domain
# controller, AD membership (ads), winbind, LDAP, PAM, ACL/quota support,
# printing (CUPS) and the Python bindings -- each needs a Linux facility,
# a library not ported here or a desktop/directory service.
VERSION=4.25.1
TARBALL=https://download.samba.org/pub/samba/stable/samba-4.25.1.tar.gz
SHA256=7f367545bca69aa96897628f82f0f04d4b66b3bcb36cb978b9bb952c4b4c428d
DEPS="gnutls tdb libiconv icu zlib libxcrypt"
BUILDSYS=custom
# The build machine needs what a native Samba configure needs (gcc, perl,
# python3, flex, bison, the GnuTLS development files): see _smb_native.
#
# pidl needs Parse::Yapp on the build machine; it is fetched from CPAN
# (pinned) and installed into the native helper directory.
_YAPP_TB=https://cpan.metacpan.org/authors/id/W/WB/WBRASWELL/Parse-Yapp-1.21.tar.gz
_YAPP_SHA=3810e998308fba2e0f4f26043035032b027ce51ce5c8a52a8b8e340ca65f13e5
_SMB_NATIVE="$BUILD_DIR/native-samba-$VERSION"
_smb_yapp() {
  [ -x "$_SMB_NATIVE/yapp/bin/yapp" ] && return 0
  local f="$WS/dist/$(basename "$_YAPP_TB")"
  [ -f "$f" ] || curl -fsSL -o "$f" "$_YAPP_TB" || return 1
  echo "$_YAPP_SHA  $f" | sha256sum -c --status || { echo "!! checksum: $f"; return 1; }
  rm -rf "$_SMB_NATIVE/yapp-src" && mkdir -p "$_SMB_NATIVE/yapp-src" &&
  tar xf "$f" -C "$_SMB_NATIVE/yapp-src" --strip-components=1 && cd "$_SMB_NATIVE/yapp-src" &&
  env PATH=/usr/bin:/bin perl Makefile.PL INSTALL_BASE="$_SMB_NATIVE/yapp" && env PATH=/usr/bin:/bin make install
}
# Heimdal's asn1_compile and compile_et run while Samba builds; waf builds
# them with the target compiler. A native configure of the same source
# builds just those two (patches/0001 makes the cross build use them via
# SAMBA_NATIVE_HEIMDAL_TOOLS).
_smb_native() {
  [ -x "$_SMB_NATIVE/src/bin/default/third_party/heimdal_build/asn1_compile" ] &&
  [ -x "$_SMB_NATIVE/src/bin/default/third_party/heimdal_build/compile_et" ] && return 0
  rm -rf "$_SMB_NATIVE/src" && cp -a "$S" "$_SMB_NATIVE/src" && cd "$_SMB_NATIVE/src" &&
  env -i HOME="$HOME" PATH="/usr/bin:/bin:$_SMB_NATIVE/yapp/bin" PERL5LIB="$_SMB_NATIVE/yapp/lib/perl5" \
    ./configure --without-ad-dc --disable-python --without-ads --without-ldap --without-winbind \
      --without-pam --without-acl-support --without-quotas --without-systemd --without-libarchive \
      --without-gpgme --disable-cups --disable-iprint --disable-avahi --disable-glusterfs \
      --disable-cephfs --disable-cephrgw --without-json --without-ldb-lmdb --without-regedit \
      --disable-spotlight --bundled-libraries=ALL --without-cluster-support --without-utmp \
      --without-fam --without-dmapi --without-automount &&
  env -i HOME="$HOME" PYTHONHASHSEED=1 PATH="/usr/bin:/bin:$_SMB_NATIVE/yapp/bin" PERL5LIB="$_SMB_NATIVE/yapp/lib/perl5" \
    buildtools/bin/waf build --targets=asn1_compile,compile_et -j"$(nproc)"
}
# The cross build. Notes:
#  - configure questions that need a test program run are answered from
#    ports/samba/cross-answers-<arch>.txt, recorded on QNX targets (see
#    the comments there); waf builds in the source tree, so in a copy.
#  - staging headers as -isystem (through qcc's -Wc,): after Samba's and
#    Heimdal's own include directories (libmd's staged md5.h/sha.h would
#    shadow Heimdal's hcrypto ones), before the SDP's (its ICU 67 headers
#    would shadow the staged ICU).
#  - -lregex and -lfsnotify: regcomp() and inotify are not in QNX's libc
#    (hazards 2, 64).
#  - --enable-fhs: required for a /usr/local prefix; state under
#    /usr/local/var, configuration in /usr/local/etc/samba.
#  - tdb is the ported one (ports/tdb); talloc, tevent, ldb, popt and
#    cmocka are Samba's bundled private copies (lib/samba, *-private-samba).
#  - no gettext: libintl's printf replacements were not linked into
#    Heimdal's com_err; messages stay English.
#  - readline is not used: its 8.2 headers fail Samba's strict-prototypes
#    probe, so smbclient reads commands without line editing.
_smb_env() {
  env CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CFLAGS="-O2 -Wc,-isystem,$STAGE_SYSROOT$PREFIX/include" \
    LDFLAGS="-L$STAGE_SYSROOT$PREFIX/lib -Wl,-rpath-link,$STAGE_SYSROOT$PREFIX/lib -lsocket -lregex -lfsnotify" \
    PYTHONHASHSEED=1 PATH="$PATH:$_SMB_NATIVE/yapp/bin" PERL5LIB="$_SMB_NATIVE/yapp/lib/perl5" \
    SAMBA_NATIVE_HEIMDAL_TOOLS="$_SMB_NATIVE/src/bin/default/third_party/heimdal_build" "$@"
}
do_build() {
  mkdir -p "$_SMB_NATIVE" && ( _smb_yapp ) && ( _smb_native ) || return 1
  rm -rf "$B" && cp -a "$S" "$B" && cd "$B" &&
  cp "$WS/ports/samba/cross-answers-$QNX_ARCH.txt" "$B/cross-answers.txt" &&
  _smb_env ./configure --prefix="$PREFIX" --enable-fhs --sysconfdir="$PREFIX/etc" --localstatedir="$PREFIX/var" \
    --cross-compile --cross-answers="$B/cross-answers.txt" --hostcc=gcc \
    --disable-python --without-ad-dc --without-ads --without-ldap --without-winbind --without-pam \
    --without-acl-support --without-quotas --without-systemd --without-libarchive --without-gpgme \
    --disable-cups --disable-iprint --disable-avahi --disable-glusterfs --disable-cephfs --disable-cephrgw \
    --without-fam --without-automount --without-dmapi --without-utmp --without-json --without-regedit \
    --without-winexe --disable-spotlight --without-ldb-lmdb --without-libunwind --without-lttng \
    --without-cluster-support --without-kernel-keyring --without-systemd-userdb --disable-rpath \
    --disable-rpath-install --bundled-libraries='!tdb,talloc,tevent,ldb,popt,cmocka' \
    --with-shared-modules='!vfs_snapper' --with-libiconv="$STAGE_SYSROOT$PREFIX" --without-gettext &&
  _smb_env make -j"$(nproc)"
}
# The state directories Samba creates only one level of (it does not
# mkdir -p): empty ones are not packaged, so each gets a placeholder and a
# packaged samba starts with its compiled-in paths.
do_install() { cd "$B" && _smb_env make install DESTDIR="$STAGE" || return 1
  local d; for d in log/samba lib/samba lib/samba/private lock/samba cache/samba run/samba; do
    mkdir -p "$STAGE$PREFIX/var/$d" && : > "$STAGE$PREFIX/var/$d/.keep" || return 1; done
  chmod 700 "$STAGE$PREFIX/var/lib/samba/private"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
