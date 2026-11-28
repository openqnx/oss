# oss-network-plugin 1.0

> Xfce panel plugin "Network": connection state, addresses, gateway, DNS, DHCP renewal

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | see recipe.sh |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | [`gtk3`](../gtk3/) [`glib`](../glib/) [`xfce4-panel`](../xfce4-panel/) |
| Run-time dependencies | [`at-spi2-core`](../at-spi2-core/) [`cairo`](../cairo/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`libxfce4util`](../libxfce4util/) [`pango`](../pango/) [`xfce4-panel`](../xfce4-panel/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_oss_network_plugin`](../../tests/src/) |

## Install and remove on a target

Not published as a package yet: build it as below, then `scripts/qnxpkg` makes `oss-network-plugin` and `oss-network-plugin-dev`, which install and remove like every other package ([INSTALL.md](../../INSTALL.md), [docs/packages.md](../../docs/packages.md)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild oss-network-plugin` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (66 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild gtk3 glib xfce4-panel
```

<details><summary>Full build chain (66)</summary>

[`at-spi2-core`](../at-spi2-core/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dbus`](../dbus/) [`exo`](../exo/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`garcon`](../garcon/) [`gdbm`](../gdbm/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`gtk-layer-shell`](../gtk-layer-shell/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libepoxy`](../libepoxy/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfce4ui`](../libxfce4ui/) [`libxfce4util`](../libxfce4util/) [`libxfce4windowing`](../libxfce4windowing/) [`libxfixes`](../libxfixes/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxtst`](../libxtst/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xfce4-panel`](../xfce4-panel/) [`xfconf`](../xfconf/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

Fetched by the recipe's `do_fetch` hook:

```bash
do_fetch() { mkdir -p "$SRCROOT" && cp -a "$RECIPE_DIR/oss-network-plugin/files/." "$SRCROOT/"; }
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/oss-network-plugin; B=build/$QNX_ARCH/oss-network-plugin; mkdir -p $B
```

```bash
do_build() {
  mkdir -p "$B" && cd "$B" && \
  qcc -V"$QCC_TARGET" -D_QNX_SOURCE -O2 -Wall -shared -fPIC -DPACKAGE_LOCALE_DIR="\"$PREFIX/share/locale\"" \
      -o liboss-network.so "$S/oss-network-plugin.c" "$S/netinfo.c" \
      $(pkg-config --cflags --libs gtk+-3.0 gio-unix-2.0 libxfce4panel-2.0) -lsocket && \
  qcc -V"$QCC_TARGET" -D_QNX_SOURCE -O2 -Wall -o oss-network-info "$S/oss-network-info.c" "$S/netinfo.c" \
      $(pkg-config --cflags --libs glib-2.0) -lsocket
}
```

```bash
do_install() {
  install -Dm755 "$B/liboss-network.so" "$STAGE$PREFIX/lib/xfce4/panel/plugins/liboss-network.so" && \
  install -Dm644 "$S/oss-network.desktop" "$STAGE$PREFIX/share/xfce4/panel/plugins/oss-network.desktop" && \
  install -Dm755 "$B/oss-network-info" "$STAGE$PREFIX/bin/oss-network-info"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- oss-network-plugin: the network indicator of the desktop panel -- the place of Ubuntu's network icon at the top right. A "Network" plugin for the Xfce panel, written for this tree (files/, C and GTK 3): an icon for the state of the connection (connected; connected without a default route; disconnected; no network -- also when io-sock does not answer or there is no interface), a tooltip with the interface, its addresses, the gateway and the DNS servers, and on a click a window with every interface (state, hardware address, IPv4/IPv6, gateway, DHCP lease) and "Renew DHCP Lease" (dhcpcd -N, run in the background, its result on a line of the window; root sessions only, since dhcpcd's control socket is root's, and only on an interface the running dhcpcd holds a lease for). Not -n: that also makes dhcpcd reload its configuration -- from the default file -- and it then took on lo0, enc0, pflog0 and pfsync0 too (seen on the QEMU image). What it reads, without privileges (QNX's io-sock speaks the BSD interfaces): getifaddrs() for interfaces, addresses and link state (if_data), a routing socket for the default route (RTM_GET) and for changes -- it refreshes on every routing message, and every 10 s besides -- and dhcpcd -U for the lease (dhcpcd's unprivileged socket). bin/oss-network-info prints the same (--dump) and counts routing messages (--events N), for tests. Wi-Fi [unverified: no Wi-Fi device in QEMU; to check on a Raspberry Pi 5]: an IEEE 802.11 interface gets the wireless icons, and its network name and state come from `wpa_cli -i <if> status`. Choosing a network, static addresses and other connection management are not part of it.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH oss-network-plugin`.

### 5. What it installs

3 files under `/usr/local`, 1 programs.

```
/usr/local/bin/oss-network-info
```

The complete list is `stage/<arch>/.manifest/oss-network-plugin` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa oss-network-plugin
ls dists/8.0/*/ | grep '^oss-network-plugin-'
```

This writes `oss-network-plugin-<ver>-r0.tar.gz` (run time) and `oss-network-plugin-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add oss-network-plugin
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/oss-network-plugin-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/oss-network-plugin-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_oss_network_plugin`](../../tests/src/) links oss-network-plugin the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-oss-network-plugin`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-oss-network-plugin
/usr/local/bin/qnx-port-tests/test_oss_network_plugin
```

```
== oss-network-plugin ==
  ok   ...
PASS oss-network-plugin
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_oss_network_plugin` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_oss_network_plugin`](../../tests/src/) -- a program linked against oss-network-plugin the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> oss-network-plugin`.

## Versions

This recipe builds **1.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add oss-network-plugin=<version>`), sources in `ported/oss-network-plugin/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates oss-network-plugin`; port one with `scripts/port-update oss-network-plugin`.

<details><summary>recipe.sh</summary>

```bash
# oss-network-plugin: the network indicator of the desktop panel -- the place
# of Ubuntu's network icon at the top right. A "Network" plugin for the Xfce
# panel, written for this tree (files/, C and GTK 3): an icon for the state
# of the connection (connected; connected without a default route;
# disconnected; no network -- also when io-sock does not answer or there is
# no interface), a tooltip with the interface, its addresses, the gateway and
# the DNS servers, and on a click a window with every interface (state,
# hardware address, IPv4/IPv6, gateway, DHCP lease) and "Renew DHCP Lease"
# (dhcpcd -N, run in the background, its result on a line of the window;
# root sessions only, since dhcpcd's control socket is root's, and only on an
# interface the running dhcpcd holds a lease for). Not -n: that also makes
# dhcpcd reload its configuration -- from the default file -- and it then
# took on lo0, enc0, pflog0 and pfsync0 too (seen on the QEMU image).
# What it reads, without privileges (QNX's io-sock speaks the BSD interfaces):
# getifaddrs() for interfaces, addresses and link state (if_data), a routing
# socket for the default route (RTM_GET) and for changes -- it refreshes on
# every routing message, and every 10 s besides -- and dhcpcd -U for the
# lease (dhcpcd's unprivileged socket). bin/oss-network-info prints the same
# (--dump) and counts routing messages (--events N), for tests.
# Wi-Fi [unverified: no Wi-Fi device in QEMU; to check on a Raspberry Pi 5]:
# an IEEE 802.11 interface gets the wireless icons, and its network name and
# state come from `wpa_cli -i <if> status`. Choosing a network, static
# addresses and other connection management are not part of it.
VERSION=1.0
DEPS="gtk3 glib xfce4-panel"
RUNTIME_DEPS="xfce4-panel adwaita-icon-theme"
BUILDSYS=custom
do_fetch() { mkdir -p "$SRCROOT" && cp -a "$RECIPE_DIR/oss-network-plugin/files/." "$SRCROOT/"; }
do_build() {
  mkdir -p "$B" && cd "$B" && \
  qcc -V"$QCC_TARGET" -D_QNX_SOURCE -O2 -Wall -shared -fPIC -DPACKAGE_LOCALE_DIR="\"$PREFIX/share/locale\"" \
      -o liboss-network.so "$S/oss-network-plugin.c" "$S/netinfo.c" \
      $(pkg-config --cflags --libs gtk+-3.0 gio-unix-2.0 libxfce4panel-2.0) -lsocket && \
  qcc -V"$QCC_TARGET" -D_QNX_SOURCE -O2 -Wall -o oss-network-info "$S/oss-network-info.c" "$S/netinfo.c" \
      $(pkg-config --cflags --libs glib-2.0) -lsocket
}
do_install() {
  install -Dm755 "$B/liboss-network.so" "$STAGE$PREFIX/lib/xfce4/panel/plugins/liboss-network.so" && \
  install -Dm644 "$S/oss-network.desktop" "$STAGE$PREFIX/share/xfce4/panel/plugins/oss-network.desktop" && \
  install -Dm755 "$B/oss-network-info" "$STAGE$PREFIX/bin/oss-network-info"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
