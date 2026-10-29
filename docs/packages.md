# Prebuilt packages and `oss-get`

Every component built here is also published as a package a QNX target can
download and install directly — the same idea as
[oss.qnx.com](https://oss.qnx.com/), served from this repository's GitHub
releases (or from any web server of your own).

## Quick start: apk on a target, from the GitHub releases

From a Linux host that can reach the target over SSH (root):

```bash
git clone https://github.com/openqnx/oss && cd oss
./scripts/target-bootstrap <target-ip>        # once per target
```

Then on the target:

```sh
. /data/oss/etc/oss-env.sh      # PATH and LD_LIBRARY_PATH (put it in your profile)
apk update
apk search json
apk add mosquitto               # + its run-time dependencies, signature-checked
mosquitto_pub --help
apk upgrade                     # everything, apk-tools included
apk del mosquitto
```

`target-bootstrap` is needed once because a stock QNX 8.0 image has no CA
certificates — its OpenSSL looks for `/usr/ssl/cert.pem`, which neither the
SDP nor the image ships — so nothing on the board can verify
`https://github.com` yet (curl error 60, apk "server certificate not
trusted"). It copies `target/oss-get`, `target/apk-setup`, the signing public
key (`keys/*.rsa.pub`) and a CA bundle to `/data/oss-setup` and runs
`apk-setup` there, which:

1. bootstraps apk-tools with `oss-get` into a scratch directory;
2. writes `/data/oss/etc/apk/repositories` and installs the key in
   `/data/oss/etc/apk/keys/`;
3. lets that apk install `apk-tools` and `ca-certificates` into `/data/oss`, so
   apk owns (and upgrades) itself and verifies HTTPS with the installed CA
   bundle from then on;
4. writes `/data/oss/bin/apk` (apk on the root `/data/oss` with its libraries
   and CA bundle), `/data/oss/etc/oss-env.sh`, and links `/usr/local` to
   `/data/oss/usr/local` (`ln -sP`, until reboot — add
   `ln -sP /data/oss/usr/local /usr/local` to a startup script to keep it).

Without a host, copy the same files to the board any way you like (USB,
`scp` from the board) and run `sh apk-setup -c <ca-bundle.pem> -k <key.pub>`.
`apk-setup -h` lists the options: `-R` another root, `-r` another repository
(your own mirror), `-u` an unsigned repository.

The repository the target uses:

```
https://github.com/openqnx/oss/releases/download/apk-8.0-<arch>/APKINDEX.tar.gz
```

with `<arch>` `x86_64` or `aarch64`. Use one package manager per root:
`apk-setup` refuses a root that `oss-get` already installed packages into.

Tested on x86_64 and aarch64le mkqnximage VMs against a local stand-in for
GitHub (HTTPS, a 302 redirect to another host, flat release assets, signed
index), and apk's and curl's HTTPS + redirect against a real
github.com release asset. Not yet tested against the real openqnx/oss
releases, which do not exist until they are published.

## oss-get

`target/oss-get` is a single POSIX shell script. It uses only what a stock
QNX 8.0 image already has: `sh`, `curl`, `tar`, `gzip` and `openssl` — but
for `https://` its curl needs a CA bundle the stock image lacks: set
`SSL_CERT_FILE=<bundle>` until `oss-get install ca-certificates` has put one
in `/data/oss/usr/local/ssl/cert.pem`, which oss-get then uses by itself.

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
# copy it to the board once (from the host: scp target/oss-get root@<target>:/data/)
chmod +x /data/oss-get
export SSL_CERT_FILE=/data/cacert.pem # first time only, see above

/data/oss-get update                 # fetch the index for this architecture
/data/oss-get install ca-certificates
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
./scripts/qnxpkg mosquitto           # -> dists/8.0/{aarch64,x86_64}/
./scripts/qnxpkg --all               # everything that has been built
```

`dists/8.0/<arch>/` then holds, per package:

```
<name>-<ver>-r0.tar.gz        what oss-get downloads
<name>-<ver>-r0.apk           the same files as an APK v2 package
INDEX                         name|version|depends|file|sha256|size|description
APKINDEX.tar.gz               the index apk-tools reads
```

Serve it with `scripts/serve-dists` (or any web server pointed at
`dists/8.0`) — see [Serving your own build](#serving-your-own-build-from-dists).

### APK packages

The `.apk` files and `APKINDEX.tar.gz` follow the APK v2 format that
repo.oss.qnx.com uses, so a target that already runs `apk` can use this
repository too. A target that does not can get it from here:
`oss-get install apk-tools` (the [apk-tools](../ports/apk-tools/README.md)
port, 3.0.0_rc5 as in the QNX Everywhere image).

```sh
oss-get install apk-tools && eval $(oss-get env)   # apk needs libapk from /usr/local/lib
R=/data/apkroot; mkdir -p $R
apk --root $R --initdb --allow-untrusted -X http://<host>:8000 add mosquitto
apk --root $R --allow-untrusted -X http://<host>:8000 info
apk --root $R --allow-untrusted -X http://<host>:8000 del mosquitto
```

How `apk` finds the index:

| Repository given as | `apk` fetches |
|---|---|
| `http://<host>:8000` | `http://<host>:8000/<arch>/APKINDEX.tar.gz` — it appends the architecture itself, so give the `dists/8.0` level, not `.../x86_64` |
| `http://<host>:8000/x86_64/APKINDEX.tar.gz` | that index; packages are fetched next to it. Use this form for a flat layout such as the GitHub releases below |

Either form also goes in `<root>/etc/apk/repositories`, one per line, after
which `apk update` / `apk add` need no `-X`.

Tested on both architectures (x86_64 and aarch64le mkqnximage VMs): `add`
with dependencies resolved from the index, `info`, `info -L`, `search`,
`del` (dependencies purged with it), `upgrade`, `fix`, and a 23-package
`add emacs`. The program installed into the root runs.

Sign the repository with an RSA key (`--key`, abuild-keygen style) and
install the public key on the target in `<root>/etc/apk/keys/`; the key's
file name must match the one used to sign (`.SIGN.RSA.<name>.pub`). Unsigned,
`apk` needs `--allow-untrusted`. With a signed repository and no matching key,
`apk` ignores the index ("no such package") instead of installing anything.

```bash
openssl genrsa -out oss.rsa 2048 && openssl rsa -in oss.rsa -pubout -out oss.rsa.pub
./scripts/qnxpkg --key oss.rsa --all          # -o DIR to write somewhere else than repo/
# on the target: cp oss.rsa.pub <root>/etc/apk/keys/
```

## Serving your own build from `dists/`

Everything `scripts/qnxpkg` writes lands in `dists/` (not tracked by git):

```
dists/8.0/aarch64/          dists/8.0/x86_64/          one repository per architecture
  <pkg>-<ver>-r0.apk          apk packages + APKINDEX.tar.gz (signed with --key)
  <pkg>-<ver>-r0.tar.gz       oss-get packages + INDEX
dists/8.0/tests/<arch>/     the port tests: qnx-test-<name>, qnx-port-tests (qnxpkg --tests)
```

It can be served at any time, as it is, to targets on the network:

```bash
./scripts/qnxpkg --key keys/private/<key>.rsa --all --tests   # (re)package what is built
./scripts/serve-dists                                         # http://<host>:8000, Ctrl-C to stop
./scripts/target-bootstrap <target> -r http://<host>:8000     # once per target
```

and then on the target:

```sh
. /data/oss/etc/oss-env.sh
apk update
apk add mosquitto                       # + its run-time dependencies, signature-checked
apk add qnx-test-mosquitto qnx-port-tests
/usr/local/bin/qnx-port-tests/test_mosquitto      # one test
/usr/local/bin/qnx-port-tests/run-on-target.sh    # every installed test
```

`target-bootstrap -r http://<host>:8000` makes `apk-setup` register both
`http://<host>:8000` (apk appends `/<arch>/APKINDEX.tar.gz`) and
`http://<host>:8000/tests` in `/data/oss/etc/apk/repositories`; `-T none`
leaves the tests out. `oss-get` reads the same tree:
`OSS_REPO=http://<host>:8000 oss-get install mosquitto`.

Each test package's dependencies are computed like any package's (from its
binary's `DT_NEEDED`), plus what the test loads at run time (GStreamer
plugins, Python's standard library, the TensorFlow Lite, PyTorch and ROS 2
prefixes), so `apk add qnx-test-<name>` brings everything the test needs.

Tested on both architectures (x86_64 and aarch64le VMs).

## Publishing to GitHub releases

GitHub allows at most 1000 assets per release, each under 2 GiB, and assets
are flat (no directories). `dists/8.0/<arch>` holds about 1640 files, so each
architecture gets three releases:

| Release | Assets | Read by |
|---|---|---|
| `packages-8.0-<arch>` | `<pkg>-<ver>.tar.gz`, `INDEX` | `oss-get` (its default `OSS_REPO`) |
| `apk-8.0-<arch>` | `<pkg>-<ver>.apk`, `APKINDEX.tar.gz`, the public key | `apk` (`.../apk-8.0-<arch>/APKINDEX.tar.gz`) |
| `tests-8.0-<arch>` | the port tests `qnx-test-<name>` (`.apk`, `.tar.gz`, both indexes) | `apk`, as a second repository |

```bash
source ./env.sh x86_64
./scripts/qnxpkg --key keys/private/<key>.rsa --all --tests   # signed; see APK packages above
./scripts/publish-github-release                     # stage in dist/release/ and check limits
./scripts/publish-github-release --upload --prune    # gh release create/upload; drop old versions
```

Without `--upload` nothing leaves the machine. With it, `gh` (logged in, with
write access to the repository) creates any missing release and uploads the
packages first and the index last, so a target never sees an index naming a
package that is not there yet; `--prune` then deletes assets no longer in
`dists/` (superseded versions). `--repo owner/name` publishes to a fork.

The signing key: the public half is `keys/<name>.rsa.pub` (tracked, and what
`target-bootstrap` installs and `apk-setup` downloads by default); the
private half stays in `keys/private/` (ignored by git — keep a copy somewhere
safe; a lost key means re-signing everything with a new one and every target
installing the new public key).
