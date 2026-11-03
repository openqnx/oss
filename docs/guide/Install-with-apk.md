# Install with apk

apk resolves dependencies, checks signatures, upgrades and removes. Do the [base image](Base-QEMU-Images)
steps first, including section 5 (the variables and the `/usr/local` link).

## 1. A temporary apk, once

apk is a package too. Unpack it by hand once, then let it install itself properly. apk-tools needs
`zlib`, `zstd`, `openssl` and `ca-certificates`. Each is in the half of the release that its name's
first letter belongs to.

```sh
qnx# mkdir -p /data/boot/root && cd /data/boot
qnx# curl -fsSL -o INDEX     $GH_RELEASES/packages-8.0-$ARCH/INDEX       # names 0-9, a-l
qnx# curl -fsSL -o INDEX-m-z $GH_RELEASES/packages-8.0-$ARCH-m-z/INDEX   # names m-z
qnx# for PKG in zlib zstd openssl ca-certificates apk-tools; do
       case $PKG in [0-9a-l]*) SHARD= ;; *) SHARD=-m-z ;; esac
       PKG_FILE=$(grep -m1 "^$PKG|" INDEX$SHARD | cut -d'|' -f4)
       PKG_SHA256=$(grep -m1 "^$PKG|" INDEX$SHARD | cut -d'|' -f5)
       curl -fsSLO $GH_RELEASES/packages-8.0-$ARCH$SHARD/$PKG_FILE
       [ "$(openssl dgst -sha256 $PKG_FILE | sed 's/.*= *//')" = "$PKG_SHA256" ] && echo "ok  $PKG_FILE" || echo "BAD $PKG_FILE"
       gzip -dc $PKG_FILE | (cd /data/boot/root && tar -xf -)
     done
qnx# ls /data/boot/root/usr/local/bin/apk
```

Every line must say `ok`.

## 2. Repositories, signing key, apk

```sh
qnx# mkdir -p /data/oss/etc/apk/keys
qnx# echo "$GH_RELEASES/apk-8.0-$ARCH/APKINDEX.tar.gz"       >  /data/oss/etc/apk/repositories
qnx# echo "$GH_RELEASES/apk-8.0-$ARCH-m-z/APKINDEX.tar.gz"   >> /data/oss/etc/apk/repositories
qnx# echo "$GH_RELEASES/tests-8.0-$ARCH/APKINDEX.tar.gz"     >> /data/oss/etc/apk/repositories
qnx# echo "$GH_RELEASES/tests-8.0-$ARCH-m-z/APKINDEX.tar.gz" >> /data/oss/etc/apk/repositories
qnx# curl -fsSL -o /data/oss/etc/apk/keys/openqnx-oss-20261029.rsa.pub \
       https://raw.githubusercontent.com/openqnx/oss/main/keys/openqnx-oss-20261029.rsa.pub
qnx# BOOT_APK=/data/boot/root/usr/local
qnx# LD_LIBRARY_PATH=$BOOT_APK/lib:/data/oss/qnx-runtime/lib $BOOT_APK/bin/apk --root /data/oss --force-no-chroot --initdb add apk-tools ca-certificates
qnx# cd /; rm -rf /data/boot
```

## 3. Using apk (at every login, after the base settings)

```sh
qnx# alias apk='/usr/local/bin/apk --root /data/oss --force-no-chroot'
qnx# export SSL_CERT_FILE=/usr/local/ssl/cert.pem    # now the one from the ca-certificates package
qnx# apk update
```

`--force-no-chroot` lets package triggers run (for example glib compiling GSettings schemas on install).

## 4. Particular packages only

```sh
qnx# apk search -v curl                     # names and versions
qnx# apk add curl                           # with its dependencies
qnx# apk add curl=<version>                 # one version (apk search -v, or the package site)
qnx# apk add curl-dev                       # headers, pkg-config, static libraries
qnx# apk info -L curl                       # the files it installed
qnx# apk del curl                           # remove (and dependencies nothing else needs)
qnx# curl --version
```

Older revisions bring matching dependencies. For example, `apk add nodejs=24.14.1-r0` comes with
ada 3.4.4, and the newest revision comes with ada 4.0.0. A package's page on the
[package site](https://openqnx.github.io/oss/) lists the dependency versions each revision works with.

## 5. Every package at once

Make the image with the large `/data` ([Base QEMU images](Base-QEMU-Images), section 2) first.

```sh
qnx# mkdir -p /data/oss/dl && cd /data/oss/dl
qnx# curl -fsSL $GH_RELEASES/packages-8.0-$ARCH/INDEX $GH_RELEASES/packages-8.0-$ARCH-m-z/INDEX \
       | grep -v '^#' | cut -d'|' -f1 | sort -u \
       | grep -vxE 'sdl2-compat(-dev)?' > all-names.txt     # sdl2-compat cannot be installed with sdl2
qnx# wc -l all-names.txt
qnx# apk update
qnx# apk add $(cat all-names.txt)                           # takes long, aarch64 especially
qnx# apk info | wc -l
```

If it is interrupted, run the same `apk add` again. Remove everything: `apk del $(cat all-names.txt)`.

## 6. Run a package's test

```sh
qnx# apk add qnx-test-curl qnx-port-tests
qnx# QNX_PORT_TESTS_VERBOSE=1 /usr/local/bin/qnx-port-tests/run-on-target.sh test_curl
```

Every check says `ok`, and the run ends with `PASS test_curl` and `passed: 1   failed: 0`. The package
site shows the same test's recorded log for comparison.

## 7. Graphical programs (image with `--graphics=yes`)

```sh
qnx# apk add oss-desktop                    # labwc, swaybg, the Xfce panel, terminal, file manager
qnx# nohup oss-desktop > /tmp/oss-desktop.out 2>&1 &
qnx# export XDG_RUNTIME_DIR=/tmp/xdg-runtime-0 WAYLAND_DISPLAY=wayland-0
qnx# apk add geany && geany &               # a GTK program on the desktop
qnx# apk add qdirstat && QT_QPA_PLATFORM=qnx qdirstat &   # Qt draws on QNX Screen directly
qnx# apk add mame && mame &                 # SDL2 programs use QNX Screen
```

Screenshots of what this looks like: [docs/screenshots](https://github.com/openqnx/oss/blob/main/docs/screenshots/README.md).
