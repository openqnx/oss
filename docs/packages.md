# Prebuilt packages and `oss-get`

Every component built here is also published as a package a QNX target can
download and install directly — the same idea as
[oss.qnx.com](https://oss.qnx.com/), served from this repository's GitHub
releases (or from any web server of your own).

## On the target

`target/oss-get` is a single POSIX shell script. It uses only what a stock
QNX 8.0 image already has: `sh`, `curl`, `tar`, `gzip` and `openssl`.

The packages themselves expect the QNX 8.0 run-time libraries a full image
carries, including the Linux-compatibility ones every port links
(`libeventfd.so.1`, `libepoll.so.1`, `libsignalfd.so.1`, `libtimerfd.so.1`,
`libfsnotify.so.1`, plus `libsocket`, `libregex`, `libm`), and the audio
ones link `libasound.so.5`. They are part of QNX, not of these packages. A trimmed image without them (mkqnximage's
default list, for one) fails every program at load with `Could not load
library libeventfd.so.1`; add them to the image from your SDP
(`target/qnx/<arch>/usr/lib`) — for mkqnximage, with
[`tests/vm/system_files.custom`](../tests/vm/system_files.custom).

```sh
# copy it to the board once
curl -fsSLo /data/oss-get https://raw.githubusercontent.com/openqnx/oss/main/target/oss-get
chmod +x /data/oss-get

/data/oss-get update                 # fetch the index for this architecture
/data/oss-get search json            # what is there
/data/oss-get info mosquitto         # version, dependencies, size
/data/oss-get install mosquitto      # + its run-time dependencies, checksummed
/data/oss-get link                   # as root: /usr/local -> /data/oss/usr/local
/data/oss-get installed
/data/oss-get remove mosquitto
```

| Setting | Default | |
|---|---|---|
| `OSS_REPO` | `https://github.com/openqnx/oss/releases/download/packages-8.0-@ARCH@` | `@ARCH@` becomes `aarch64` or `x86_64`; without it `/<arch>` is appended |
| `OSS_ROOT` | `/data/oss` | where packages are unpacked |
| `OSS_ARCH` | from `uname -m` | override the detected architecture |

Settings can also go in `/data/oss/etc/oss-get.conf` (shell syntax).

### Why `/usr/local` and `oss-get link`

Everything is built for the prefix `/usr/local`: data directories, plugin
paths and the terminfo database are compiled into the binaries. On images
where `/` is the read-only IFS, `oss-get` unpacks into `$OSS_ROOT/usr/local`
and `oss-get link` maps `/usr/local` onto it with a QNX path-manager link
(`ln -sP`). The link lasts until reboot; add it to a startup script to keep
it. Without the link, `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`
for the unpacked tree, which is enough for most libraries.

### Packages

Each component becomes two packages:

| Package | Contains |
|---|---|
| `<name>` | shared libraries, programs, run-time data |
| `<name>-dev` | headers, pkg-config files, CMake package files, static libraries, man pages |

A package's dependencies are **computed from its binaries**: each `DT_NEEDED`
entry is mapped to the component that installed that soname. Libraries of
the QNX SDP target are never dependencies. So `oss-get install` pulls in
exactly what the program loads — see the run-time column in
[DEPENDENCIES.md](../DEPENDENCIES.md).

## Building packages

```bash
source ./env.sh aarch64le
./scripts/qnxbuild mosquitto         # installs into pkgroot/<arch>/mosquitto too
./scripts/qnxpkg mosquitto           # -> repo/8.0/{aarch64,x86_64}/
./scripts/qnxpkg --all               # everything that has been built
```

`repo/8.0/<arch>/` then holds, per package:

```
<name>-<ver>-r0.tar.gz        what oss-get downloads
<name>-<ver>-r0.apk           the same files as an APK v2 package
INDEX                         name|version|depends|file|sha256|size|description
APKINDEX.tar.gz               the index apk-tools reads
```

Serve it with anything — `python3 -m http.server -d repo/8.0 8000` is enough
for a lab — and point `OSS_REPO` at it.

### APK packages

The `.apk` files and `APKINDEX.tar.gz` follow the APK v2 format that
repo.oss.qnx.com uses, so a target that already runs `apk` can use this
repository too. Sign them with an RSA key (`--key`, abuild-keygen style) and
install the public key on the target in `/etc/apk/keys`; unsigned, `apk`
needs `--allow-untrusted`.

```bash
openssl genrsa -out oss.rsa 2048 && openssl rsa -in oss.rsa -pubout -out oss.rsa.pub
./scripts/qnxpkg --key oss.rsa --all
```

## Publishing to GitHub releases

One release per architecture, assets flat (GitHub release assets have no
directories), which is what the `@ARCH@` in the default `OSS_REPO` expects:

```bash
for a in aarch64 x86_64; do
  gh release create packages-8.0-$a --title "QNX SDP 8.0 packages ($a)" \
     --notes "Install on target with target/oss-get" repo/8.0/$a/*
done
```
