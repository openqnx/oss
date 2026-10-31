# Getting started: install a package on QNX 8.0 and test it

Three steps, shown with two representative components:
**zstd** (standalone: needs nothing else) and **curl** (dependent: needs six
other packages). Every other package works the same way; its page on the
[package site](https://openqnx.github.io/oss/) names its dependencies, its
test and the recorded run to compare with.

| Step | What | Section |
|---|---|---|
| 1 | A QNX 8.0 target: a stock QEMU image from your SDP | [1](#1-prepare-a-target) |
| 2 | Install: **A** with `apk`, or **B** by downloading the package archive | [2A](#2a-install-with-apk) / [2B](#2b-install-by-downloading-the-archive) |
| 3 | Run the package's test, compare with the recorded log | [3](#3-test-it) |

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
itself properly:

```sh
qnx# mkdir -p /data/boot/root && cd /data/boot
qnx# curl -fsSLO $GH/packages-8.0-$A/INDEX
qnx# for p in zlib zstd openssl ca-certificates apk-tools; do
       l=$(grep "^$p|" INDEX | head -n 1); f=$(echo "$l" | cut -d'|' -f4)
       curl -fsSLO $GH/packages-8.0-$A/$f
       [ "$(openssl dgst -sha256 $f | sed 's/.*= *//')" = "$(echo "$l" | cut -d'|' -f5)" ] || echo "BAD $f"
       gzip -dc $f | (cd root && tar -xf -)
     done
qnx# mkdir -p /data/oss/etc/apk/keys
qnx# echo "$GH/apk-8.0-$A/APKINDEX.tar.gz"   >  /data/oss/etc/apk/repositories
qnx# echo "$GH/tests-8.0-$A/APKINDEX.tar.gz" >> /data/oss/etc/apk/repositories
qnx# curl -fsSL -o /data/oss/etc/apk/keys/openqnx-oss-20261029.rsa.pub \
       https://raw.githubusercontent.com/openqnx/oss/main/keys/openqnx-oss-20261029.rsa.pub
qnx# LD_LIBRARY_PATH=root/usr/local/lib:$LD_LIBRARY_PATH \
       root/usr/local/bin/apk --root /data/oss --initdb add apk-tools ca-certificates
qnx# cd / && rm -rf /data/boot
qnx# alias apk='apk --root /data/oss'; export SSL_CERT_FILE=/usr/local/ssl/cert.pem
```

(`target/apk-setup` in this repository does the same in one command.)

Then install the samples. apk fetches the dependencies, checks every
signature, and installs the test packages from the tests repository:

```sh
qnx# apk add zstd qnx-test-zstd qnx-port-tests            # standalone
qnx# apk add curl qnx-test-curl                           # dependent: brotli c-ares nghttp2 openssl zlib zstd come along
qnx# apk info -L curl                                     # what it installed
qnx# apk del curl qnx-test-curl                           # remove again
```

## 2B. Install by downloading the archive

Without apk: each package is a `.tar.gz` of files under `usr/local/`, listed
in the release's `INDEX` with its dependencies and SHA-256. Install the
dependencies first, yourself:

```sh
qnx# mkdir -p /data/oss/dl && cd /data/oss/dl
qnx# curl -fsSLO $GH/packages-8.0-$A/INDEX
qnx# grep '^curl|' INDEX | cut -d'|' -f1-3                # name|version|dependencies
curl|8.11.1-r0|brotli,c-ares,nghttp2,openssl,zlib,zstd
qnx# for p in brotli c-ares nghttp2 openssl zlib zstd curl; do        # dependencies first
       l=$(grep "^$p|" INDEX | head -n 1); f=$(echo "$l" | cut -d'|' -f4)
       curl -fsSLO $GH/packages-8.0-$A/$f
       [ "$(openssl dgst -sha256 $f | sed 's/.*= *//')" = "$(echo "$l" | cut -d'|' -f5)" ] \
         && gzip -dc $f | (cd /data/oss && tar -xf -) && echo "installed $p" || echo "BAD $f"
     done
qnx# curl --version                                       # the ported curl
```

The tests come the same way from the `tests-8.0-$A` release (its own `INDEX`):
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

## Which package, which test

- [Package site](https://openqnx.github.io/oss/): every package with its
  install command, dependencies, test and recorded log.
- [docs/target-guide.md](docs/target-guide.md): all components by kind
  (standalone first, then dependent), with their test packages.
- To build a component yourself from upstream instead: `ports/<name>/README.md`.

| Problem | Fix |
|---|---|
| `curl: (60) SSL certificate problem` | `export SSL_CERT_FILE=...` (step 1) |
| `... .so.1 not found` | the QNX run-time libraries (step 1), and `LD_LIBRARY_PATH` |
| a program cannot find its data | `/usr/local` must point to `/data/oss/usr/local` |
| `No space left on device` | `apk del` what you tested, or make `/data` larger (`--data-size`) |
