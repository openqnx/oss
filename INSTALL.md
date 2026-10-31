# Getting started: install a package on QNX 8.0 and test it

Three steps, and a fourth to undo them, shown with two representative components:
**zstd** (standalone: needs nothing else) and **curl** (dependent: needs six
other packages). Every other package works the same way; its page on the
[package site](https://openqnx.github.io/oss/) names its dependencies, its
test and the recorded run to compare with.

| Step | What | Section |
|---|---|---|
| 1 | A QNX 8.0 target: a stock QEMU image from your SDP | [1](#1-prepare-a-target) |
| 2 | Install: **A** with `apk`, or **B** by downloading the package archive | [2A](#2a-install-with-apk) / [2B](#2b-install-by-downloading-the-archive) |
| 3 | Run the package's test, compare with the recorded log | [3](#3-test-it) |
| 4 | Remove: **A** `apk del`, **B** `oss-get remove` or the file list saved at install | [4](#4-remove) |

`host$` is your Linux machine, `qnx#` a root shell on the target.

## 1. Prepare a target

You need QNX SDP 8.0 (with a license: <https://www.qnx.com/getqnx>), QEMU and
the libvirt bridge `virbr0`. Every step and option is in
[docs/qemu-test-vms.md](docs/qemu-test-vms.md); in short, for x86_64:

```bash
host$ source ~/qnx800/qnxsdp-env.sh
host$ mkdir -p ~/vm/x86 && cd ~/vm/x86
host$ mkqnximage --type=qemu --arch=x86_64 --ssh-ident=$HOME/.ssh/id_ed25519.pub \
        --data-size=4000 --data-inodes=80000 --ram=2G --cpu=4 --run=-h
host$ T=$(mkqnximage --getip)
```

(aarch64: the same with `--arch=aarch64le`, in its own directory.)

A stock image lacks two things the packages need. Copy them over once:

```bash
# a CA bundle, so the target can download from https://github.com
host$ ssh root@$T 'mkdir -p /data/oss-setup /data/oss/qnx-runtime/lib'
host$ scp /etc/ssl/certs/ca-certificates.crt root@$T:/data/oss-setup/cacert.pem
# QNX's own run-time libraries that the image does not carry (from your SDP)
host$ for l in libeventfd.so.1 libepoll.so.1 libsignalfd.so.1 libtimerfd.so.1 libasound.so.5 \
               libscreen.so.1 libEGL.so.1 libGLESv2.so.1 libbacktrace.so.1 libdrm.so.1 libdrm.so.2 libgbm.so.1; do
        f=$(ls ~/qnx800/target/qnx/x86_64/usr/lib/$l ~/qnx800/target/qnx/x86_64/lib/$l 2>/dev/null | head -n 1)
        [ -n "$f" ] && scp "$f" root@$T:/data/oss/qnx-runtime/lib/
      done
host$ ssh root@$T
```

On the target, in every shell:

```sh
qnx# A=x86_64                     # aarch64 on an aarch64 target
qnx# GH=https://github.com/openqnx/oss/releases/download
qnx# export SSL_CERT_FILE=/data/oss-setup/cacert.pem
qnx# export PATH=/usr/local/bin:/usr/local/sbin:$PATH
qnx# export LD_LIBRARY_PATH=/usr/local/lib:/data/oss/qnx-runtime/lib
```

Packages install under `/data/oss` and expect `/usr/local` to point there:

```sh
qnx# mkdir -p /data/oss/usr/local && ln -sP /data/oss/usr/local /usr/local   # again after a reboot
```

## 2A. Install with apk

apk is a package too. Unpack it once from its archive, then let it install
itself properly. The packages are in two releases per architecture (GitHub
allows 1000 files per release): `packages-8.0-$A` holds the names starting
with 0-9 or a-l, `packages-8.0-$A-m-z` the rest. Each has its own `INDEX`;
the loop below merges both and remembers which release each line came from:

```sh
qnx# mkdir -p /data/boot/root && cd /data/boot
qnx# for r in packages-8.0-$A packages-8.0-$A-m-z; do
       curl -fsSL $GH/$r/INDEX | grep -v '^#' | sed "s/\$/|$r/"
     done > INDEX                                         # each line ends with its release
qnx# for p in zlib zstd openssl ca-certificates apk-tools; do
       l=$(grep "^$p|" INDEX | head -n 1); f=$(echo "$l" | cut -d'|' -f4); r=${l##*|}
       curl -fsSLO $GH/$r/$f
       [ "$(openssl dgst -sha256 $f | sed 's/.*= *//')" = "$(echo "$l" | cut -d'|' -f5)" ] || echo "BAD $f"
       gzip -dc $f | (cd root && tar -xf -)
     done
qnx# mkdir -p /data/oss/etc/apk/keys
qnx# echo "$GH/apk-8.0-$A/APKINDEX.tar.gz"     >  /data/oss/etc/apk/repositories   # 0-9, a-l
qnx# echo "$GH/apk-8.0-$A-m-z/APKINDEX.tar.gz" >> /data/oss/etc/apk/repositories   # m-z
qnx# echo "$GH/tests-8.0-$A/APKINDEX.tar.gz"   >> /data/oss/etc/apk/repositories
qnx# curl -fsSL -o /data/oss/etc/apk/keys/openqnx-oss-20261029.rsa.pub \
       https://raw.githubusercontent.com/openqnx/oss/main/keys/openqnx-oss-20261029.rsa.pub
qnx# LD_LIBRARY_PATH=root/usr/local/lib:$LD_LIBRARY_PATH \
       root/usr/local/bin/apk --root /data/oss --initdb add apk-tools ca-certificates
qnx# cd / && rm -rf /data/boot
qnx# alias apk='apk --root /data/oss'; export SSL_CERT_FILE=/usr/local/ssl/cert.pem
```

(`target/apk-setup` in this repository does the same in one command.) apk
reads the three repositories as one: a package in one may depend on packages
in another.

A target set up before the split (November 2026) has only the
`apk-8.0-$A` line and sees no new package named m-z: add the second line
(`echo "$GH/apk-8.0-$A-m-z/APKINDEX.tar.gz" >> /data/oss/etc/apk/repositories`,
then `apk update`), or run `target/apk-setup` again. For oss-get, the new
default `OSS_REPO` names both releases; a `/data/oss/etc/oss-get.conf` that
sets `OSS_REPO` to the old release needs the `-m-z` one added (space-separated).

Then install the samples. apk fetches the dependencies, checks every
signature, and installs the test packages from the tests repository:

```sh
qnx# apk add zstd qnx-test-zstd qnx-port-tests            # standalone
qnx# apk add curl qnx-test-curl                           # dependent: brotli c-ares nghttp2 openssl zlib zstd come along
qnx# apk info -L curl                                     # what it installed
qnx# apk del curl qnx-test-curl                           # remove again (step 4)
```

## 2B. Install by downloading the archive

Without apk: each package is a `.tar.gz` of files under `usr/local/`, listed
in its release's `INDEX` with its dependencies and SHA-256. There are two
releases, `packages-8.0-$A` (names 0-9, a-l) and `packages-8.0-$A-m-z` (m-z),
so read both indexes; a package's dependencies may be in the other one.
Install the dependencies first, yourself, and keep each archive's file list
(in `/data/oss/var/lib/manual/<pkg>.list`) so that it can be removed again:

```sh
qnx# mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
qnx# curl -fsSL -o INDEX.a-l $GH/packages-8.0-$A/INDEX
qnx# curl -fsSL -o INDEX.m-z $GH/packages-8.0-$A-m-z/INDEX
qnx# { sed "s/\$/|packages-8.0-$A/" INDEX.a-l; sed "s/\$/|packages-8.0-$A-m-z/" INDEX.m-z; } |
       grep -v '^#' > INDEX                               # each line ends with its release
qnx# grep '^curl|' INDEX | cut -d'|' -f1-3                # name|version|dependencies
curl|8.11.1-r0|brotli,c-ares,nghttp2,openssl,zlib,zstd
qnx# for p in brotli c-ares nghttp2 openssl zlib zstd curl; do        # dependencies first
       l=$(grep "^$p|" INDEX | head -n 1); f=$(echo "$l" | cut -d'|' -f4); r=${l##*|}
       curl -fsSLO $GH/$r/$f
       [ "$(openssl dgst -sha256 $f | sed 's/.*= *//')" = "$(echo "$l" | cut -d'|' -f5)" ] \
         && gzip -dc $f | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/$p.list \
         && gzip -dc $f | (cd /data/oss && tar -xf -) && echo "installed $p" || echo "BAD $f"
     done
qnx# curl --version                                       # the ported curl
```

(`target/oss-get` does this for you, file lists included: `oss-get update` merges both
indexes, `oss-get install curl` installs the dependencies first.)
The tests come the same way from the `tests-8.0-$A` release (one release, its own `INDEX`):
`qnx-port-tests` (the runner) and `qnx-test-curl`. Use one method per
`/data/oss`: apk does not know about files unpacked by hand.

## 3. Test it

Each component's test is a program that uses it and checks known answers.
Run it through the runner, exactly as the recorded results were made:

```sh
qnx# QNX_PORT_TESTS_VERBOSE=1 /usr/local/bin/qnx-port-tests/run-on-target.sh test_zstd
     == zstd ==
       ok   version >= 1.5
       ok   allocate work buffers
       ok   compress succeeds
       ok   output is smaller than input
       ok   decompressed size matches
       ok   round trip is byte-identical
     PASS zstd
PASS test_zstd
--------------------------------------------
passed: 1   failed: 0
```

```sh
qnx# QNX_PORT_TESTS_VERBOSE=1 /usr/local/bin/qnx-port-tests/run-on-target.sh test_curl
```

checks TLS support, URL parsing and a real HTTP transfer over the loopback.

Compare with the recorded run, which is in
`tests/logs/<arch>/<name>-<version>.log` (`<arch>`: `x86_64` or `aarch64le`)
and on the package's page of the site, e.g.
[zstd](tests/logs/x86_64/zstd-1.5.6-r0.log) and
[curl](tests/logs/x86_64/curl-8.11.1-r0.log). These must match: every `ok`
check name, `PASS`, `passed: 1 failed: 0`. These may differ: times,
addresses and package counts in `info` lines and apk's output.

## 4. Remove

Remove with the method you installed with. Every package's page on the
[package site](https://openqnx.github.io/oss/) shows all three for that
package, with its exact file and SHA-256.

**A. apk** removes the package and every dependency nothing else needs (one
you added yourself, like `zstd` above, stays until you `apk del` it):

```sh
qnx# apk del curl qnx-test-curl                           # brotli c-ares nghttp2 openssl zlib go with it
qnx# apk del zstd qnx-test-zstd qnx-port-tests
```

**B-1. oss-get** deletes the files it listed when it installed the package.
The dependencies stay; `oss-get remove` refuses a package another installed
one still needs, so remove the dependents first:

```sh
qnx# oss-get remove curl
qnx# oss-get remove brotli c-ares nghttp2 openssl zlib zstd   # its dependencies, if nothing else needs them
qnx# oss-get installed                                    # what is left
```

**B-2. By hand**: delete exactly the files in the list saved at install
(2B), dependents first. Leave out a dependency another installed package
still needs:

```sh
qnx# cd /data/oss && for p in qnx-test-curl qnx-test-zstd qnx-port-tests \
                                 curl brotli c-ares nghttp2 openssl zlib zstd; do
       [ -f var/lib/manual/$p.list ] || continue
       while read -r f; do rm -f "$f"; done < var/lib/manual/$p.list
       rm var/lib/manual/$p.list && echo "removed $p"
     done
```

Packages without a saved list are skipped; for zstd alone:
`for p in qnx-test-zstd qnx-port-tests zstd`. Empty directories stay. A `-dev`
package (headers, pkg-config/CMake files, static libraries) is removed the
same way, before the package it belongs to.

## Which package, which test

- [Package site](https://openqnx.github.io/oss/): every package with its
  install and remove commands (apk, oss-get, by hand), dependencies, test and recorded log.
- [docs/target-guide.md](docs/target-guide.md): all components by kind
  (standalone first, then dependent), with their test packages.
- To build a component yourself from upstream instead: `ports/<name>/README.md`.

| Problem | Fix |
|---|---|
| `curl: (60) SSL certificate problem` | `export SSL_CERT_FILE=...` (step 1) |
| `... .so.1 not found` | the QNX run-time libraries (step 1), and `LD_LIBRARY_PATH` |
| a program cannot find its data | `/usr/local` must point to `/data/oss/usr/local` |
| `No space left on device` | `apk del` what you tested, or make `/data` larger (`--data-size`) |
