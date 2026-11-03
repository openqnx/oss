# Install the archives (no apk)

Every package is also a plain `.tar.gz` in the `packages-8.0-<ARCH>` releases. To install one, check
its SHA-256 from the release's `INDEX`, then unpack it under `/data/oss`. Without apk you install
the dependencies yourself, and a saved file list lets you remove exactly what you installed. Do the
[base image](Base-QEMU-Images) steps first, including section 5.

## 1. All packages at once: `install-all`

[`target/install-all`](https://github.com/openqnx/oss/blob/main/target/install-all) does the steps
of section 2 for every package. It takes the newest version of each package from both `INDEX`
files and leaves out `sdl2-compat`, which is the alternative to `sdl2`. It uses only what a default
image has (curl, openssl, sed, awk, tar, gzip).

```sh
qnx# cd /data && curl -fsSLO https://raw.githubusercontent.com/openqnx/oss/main/target/install-all
qnx# sh /data/install-all $ARCH                    # install root /data/oss by default
qnx#   ... [n/N] <name> <version> ...
qnx#   done: N installed, 0 already there, 0 failed (of N)
qnx# ONLY='zlib|zstd|curl' sh /data/install-all $ARCH   # only these (a trial run)
```

Run it again to resume, or to retry what failed: packages already installed (those with a file list)
are skipped. Make the image with the large `/data` first
([Base QEMU images](Base-QEMU-Images), section 2).

Remove everything:

```sh
qnx# cd /data/oss && for LIST_FILE in var/lib/manual/*.list; do
       while read -r FILE; do rm -f "$FILE"; done < "$LIST_FILE"; rm -f "$LIST_FILE"
     done
```

## 2. One package by hand

```sh
qnx# mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
qnx# curl -fsSL -o INDEX     $GH_RELEASES/packages-8.0-$ARCH/INDEX       # names 0-9, a-l
qnx# curl -fsSL -o INDEX-m-z $GH_RELEASES/packages-8.0-$ARCH-m-z/INDEX   # names m-z
qnx# grep -m1 '^curl|' INDEX | cut -d'|' -f3       # its dependencies: install them first, the same way
```

The five steps for one package (curl: its name starts with c, so `INDEX` and `packages-8.0-$ARCH`):

```sh
qnx# PKG=curl; INDEX_FILE=INDEX; RELEASE=packages-8.0-$ARCH   # m-z name: INDEX-m-z, packages-8.0-$ARCH-m-z
qnx# PKG_FILE=$(grep -m1 "^$PKG|" $INDEX_FILE | cut -d'|' -f4)
qnx# PKG_SHA256=$(grep -m1 "^$PKG|" $INDEX_FILE | cut -d'|' -f5)
qnx# curl -fsSLO $GH_RELEASES/$RELEASE/$PKG_FILE                           # 1. download
qnx# openssl dgst -sha256 $PKG_FILE; echo $PKG_SHA256                      # 2. the two must match
qnx# gzip -dc $PKG_FILE | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/$PKG.list   # 3. file list
qnx# gzip -dc $PKG_FILE | (cd /data/oss && tar -xf -)                      # 4. unpack
qnx# rm $PKG_FILE                                                          # 5. clean up
qnx# curl --version
```

`-dev` packages work the same way, by their own name (`PKG=curl-dev`). Remove a package:

```sh
qnx# cd /data/oss && while read -r FILE; do rm -f "$FILE"; done < var/lib/manual/curl.list
qnx# rm /data/oss/var/lib/manual/curl.list
```

## 3. After installing GTK programs

apk compiles GSettings schemas and the MIME database through triggers. Without apk, do it yourself
after installing such packages:

```sh
qnx# glib-compile-schemas /usr/local/share/glib-2.0/schemas
qnx# update-mime-database /usr/local/share/mime
```

## 4. Run a package's test

The tests are archives too, in `tests-8.0-<ARCH>` (`qnx-test-<name>`, 0-9/a-l) and
`tests-8.0-<ARCH>-m-z` (m-z, and the runner `qnx-port-tests`). Install `qnx-port-tests` and
`qnx-test-curl` with the five steps, using those releases' `INDEX`. Then:

```sh
qnx# QNX_PORT_TESTS_VERBOSE=1 /usr/local/bin/qnx-port-tests/run-on-target.sh test_curl
```
