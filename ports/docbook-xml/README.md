# docbook-xml 4.5

> DocBook XML DTDs 4.1.2 to 4.5 with XML catalogs, for validating and transforming DocBook documents offline

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://archive.docbook.org/xml/4.5/docbook-xml-4.5.zip |
| SHA-256 | `4e4e037a2b83c98c6c94818390d4bdd3f6e10f6ec62dd79188594e26190dc7b4` |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_docbook_xml`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `docbook-xml`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`docbook-xml`](https://openqnx.github.io/oss/package/x86_64/docbook-xml.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/docbook-xml.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add docbook-xml` (dependencies come along); one version: `apk add docbook-xml=<version>` | `apk del docbook-xml` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install docbook-xml` (dependencies first) | `oss-get remove docbook-xml` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-docbook-xml && /usr/local/bin/qnx-port-tests/test_docbook_xml    # its on-target test, from tests-8.0-<arch>
apk del docbook-xml qnx-test-docbook-xml
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/docbook-xml.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/docbook-xml.list
rm var/lib/manual/docbook-xml.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild docbook-xml` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

No other component has to be built first.

### 2. Get the upstream source

```bash
URL=https://archive.docbook.org/xml/4.5/docbook-xml-4.5.zip
mkdir -p dist src/docbook-xml
curl -fL -o dist/$(basename $URL) $URL
echo "4e4e037a2b83c98c6c94818390d4bdd3f6e10f6ec62dd79188594e26190dc7b4  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/docbook-xml --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/docbook-xml; B=build/$QNX_ARCH/docbook-xml; mkdir -p $B
```

```bash
do_configure() { :; }
```

```bash
do_build() { :; }
```

```bash
do_install() {
  local base="$STAGE$PREFIX/share/xml/docbook" v d
  mkdir -p "$STAGE$PREFIX/etc/xml"
  for v in $DBX_VERSIONS; do
    d="$base/xml-dtd-$v"; mkdir -p "$d"
    ( cd "$S/$v" && cp -a docbook.cat ./*.dtd ./*.mod ent "$d/" ) || return 1
    # 4.5 ships a catalog.xml; written for every version alike here
    dbx_catalog "$v" "$d"
    [ "$(grep -c '<public ' "$d/catalog.xml")" -ge 20 ] || { echo "catalog for $v: too few entries"; return 1; }
  done
  # etc/xml/docbook-xml: the versions, newest first. The ISO entity sets have
  # the same public identifiers in several versions: the newest wins.
  {
    echo '<?xml version="1.0"?>'
    echo '<!DOCTYPE catalog PUBLIC "-//OASIS//DTD Entity Resolution XML Catalog V1.0//EN" "http://www.oasis-open.org/committees/entity/release/1.0/catalog.dtd">'
    echo '<catalog xmlns="urn:oasis:names:tc:entity:xmlns:xml:catalog">'
    for v in $DBX_VERSIONS; do echo "  <nextCatalog catalog=\"file://$PREFIX/share/xml/docbook/xml-dtd-$v/catalog.xml\"/>"; done
    echo '</catalog>'
  } > "$STAGE$PREFIX/etc/xml/docbook-xml"
  # The root catalog: DocBook's identifiers delegated to the catalog above.
  {
    echo '<?xml version="1.0"?>'
    echo '<!DOCTYPE catalog PUBLIC "-//OASIS//DTD Entity Resolution XML Catalog V1.0//EN" "http://www.oasis-open.org/committees/entity/release/1.0/catalog.dtd">'
    echo '<catalog xmlns="urn:oasis:names:tc:entity:xmlns:xml:catalog">'
    local c="file://$PREFIX/etc/xml/docbook-xml" p
    for p in "-//OASIS//DTD DocBook" "-//OASIS//ELEMENTS DocBook" "-//OASIS//ENTITIES DocBook" \
             "-//OASIS//DTD XML Exchange Table Model" "ISO 8879:1986"; do
      echo "  <delegatePublic publicIdStartString=\"$p\" catalog=\"$c\"/>"
    done
    for p in "http://www.oasis-open.org/docbook/" "http://docbook.org/xml/" "https://docbook.org/xml/"; do
      echo "  <delegateSystem systemIdStartString=\"$p\" catalog=\"$c\"/>"
      echo "  <delegateURI uriStartString=\"$p\" catalog=\"$c\"/>"
    done
    echo '</catalog>'
  } > "$STAGE$PREFIX/etc/xml/catalog"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- docbook-xml: the DocBook XML DTDs 4.5, with 4.4, 4.3, 4.2 and 4.1.2 (as Alpine and QNX package them: documents in the wild declare all of these), and XML catalogs that map their public and system identifiers to the local files, so xmllint --valid, xsltproc and the GNOME documentation tools work offline. Data only. share/xml/docbook/xml-dtd-<v>/          the DTDs, modules, entity sets, and catalog.xml (written here) etc/xml/docbook-xml                     a catalog of all versions etc/xml/catalog                         the root catalog libxml2 reads by default (file:///usr/local/etc/xml/catalog)
- The older DTD versions: archive.docbook.org is where docbook.org's release files now live (the old www.docbook.org/xml/ paths answer 404).
- A catalog for one DTD version, from its SGML catalog (docbook.cat): every PUBLIC identifier to its file, and the version's canonical system URL (http://www.oasis-open.org/docbook/xml/<v>/) rewritten to this directory. URIs are relative to the catalog's own location.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH docbook-xml`.

### 5. What it installs

152 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/docbook-xml` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa docbook-xml
ls dists/8.0/*/ | grep '^docbook-xml-'
```

This writes `docbook-xml-<ver>-r0.tar.gz` (run time) and `docbook-xml-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add docbook-xml
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/docbook-xml-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/docbook-xml-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_docbook_xml`](../../tests/src/) links docbook-xml the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-docbook-xml`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-docbook-xml
/usr/local/bin/qnx-port-tests/test_docbook_xml
```

```
== docbook-xml ==
  ok   ...
PASS docbook-xml
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_docbook_xml` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_docbook_xml`](../../tests/src/) -- a program linked against docbook-xml the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> docbook-xml`.

## Versions

This recipe builds **4.5**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add docbook-xml=<version>`), sources in `ported/docbook-xml/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates docbook-xml`; port one with `scripts/port-update docbook-xml`.

<details><summary>recipe.sh</summary>

```bash
# docbook-xml: the DocBook XML DTDs 4.5, with 4.4, 4.3, 4.2 and 4.1.2 (as
# Alpine and QNX package them: documents in the wild declare all of these),
# and XML catalogs that map their public and system identifiers to the local
# files, so xmllint --valid, xsltproc and the GNOME documentation tools work
# offline. Data only.
#   share/xml/docbook/xml-dtd-<v>/          the DTDs, modules, entity sets,
#                                           and catalog.xml (written here)
#   etc/xml/docbook-xml                     a catalog of all versions
#   etc/xml/catalog                         the root catalog libxml2 reads by
#                                           default (file:///usr/local/etc/xml/catalog)
VERSION=4.5
TARBALL=https://archive.docbook.org/xml/4.5/docbook-xml-4.5.zip
SHA256=4e4e037a2b83c98c6c94818390d4bdd3f6e10f6ec62dd79188594e26190dc7b4
RUNTIME_DEPS="libxml2"
BUILDSYS=custom
# The older DTD versions: archive.docbook.org is where docbook.org's release
# files now live (the old www.docbook.org/xml/ paths answer 404).
DBX_OLD="4.4:02f159eb88c4254d95e831c51c144b1863b216d909b5ff45743a1ce6f5273090:docbook-xml-4.4.zip
4.3:23068a94ea6fd484b004c5a73ec36a66aa47ea8f0d6b62cc1695931f5c143464:docbook-xml-4.3.zip
4.2:acc4601e4f97a196076b7e64b368d9248b07c7abf26b34a02cca40eeebe60fa2:docbook-xml-4.2.zip
4.1.2:30f0644064e0ea71751438251940b1431f46acada814a062870f486c772e7772:docbkx412.zip"
DBX_VERSIONS="4.5 4.4 4.3 4.2 4.1.2"
do_fetch() {
  mkdir -p "$WS/dist" "$SRCROOT"
  local line v sum f a
  for line in "4.5:$SHA256:docbook-xml-4.5.zip" $DBX_OLD; do
    v=${line%%:*}; sum=${line#*:}; sum=${sum%%:*}; f=${line##*:}
    a="$WS/dist/docbook-xml-$v.zip"
    [ -f "$a" ] || curl -fsSL --retry 3 -o "$a" "https://archive.docbook.org/xml/$v/$f" || { rm -f "$a"; return 1; }
    echo "$sum  $a" | sha256sum -c --status || { echo "checksum mismatch: $a"; rm -f "$a"; return 1; }
    mkdir -p "$SRCROOT/$v" && ( cd "$SRCROOT/$v" && unzip -qo "$a" ) || return 1
  done
}
do_configure() { :; }
do_build() { :; }
# A catalog for one DTD version, from its SGML catalog (docbook.cat): every
# PUBLIC identifier to its file, and the version's canonical system URL
# (http://www.oasis-open.org/docbook/xml/<v>/) rewritten to this directory.
# URIs are relative to the catalog's own location.
dbx_catalog() {
  local v=$1 dir=$2
  {
    echo '<?xml version="1.0"?>'
    echo '<!DOCTYPE catalog PUBLIC "-//OASIS//DTD Entity Resolution XML Catalog V1.0//EN" "http://www.oasis-open.org/committees/entity/release/1.0/catalog.dtd">'
    echo '<catalog xmlns="urn:oasis:names:tc:entity:xmlns:xml:catalog" prefer="public">'
    tr '\n\r\t' '   ' < "$dir/docbook.cat" | grep -o 'PUBLIC *"[^"]*" *"[^"]*"' |
      sed 's/^PUBLIC *"\([^"]*\)" *"\([^"]*\)"/  <public publicId="\1" uri="\2"\/>/'
    for u in "http://www.oasis-open.org/docbook/xml/$v/" "http://docbook.org/xml/$v/" "https://docbook.org/xml/$v/"; do
      echo "  <rewriteSystem systemIdStartString=\"$u\" rewritePrefix=\"./\"/>"
      echo "  <rewriteURI uriStartString=\"$u\" rewritePrefix=\"./\"/>"
    done
    echo '</catalog>'
  } > "$dir/catalog.xml"
}
do_install() {
  local base="$STAGE$PREFIX/share/xml/docbook" v d
  mkdir -p "$STAGE$PREFIX/etc/xml"
  for v in $DBX_VERSIONS; do
    d="$base/xml-dtd-$v"; mkdir -p "$d"
    ( cd "$S/$v" && cp -a docbook.cat ./*.dtd ./*.mod ent "$d/" ) || return 1
    # 4.5 ships a catalog.xml; written for every version alike here
    dbx_catalog "$v" "$d"
    [ "$(grep -c '<public ' "$d/catalog.xml")" -ge 20 ] || { echo "catalog for $v: too few entries"; return 1; }
  done
  # etc/xml/docbook-xml: the versions, newest first. The ISO entity sets have
  # the same public identifiers in several versions: the newest wins.
  {
    echo '<?xml version="1.0"?>'
    echo '<!DOCTYPE catalog PUBLIC "-//OASIS//DTD Entity Resolution XML Catalog V1.0//EN" "http://www.oasis-open.org/committees/entity/release/1.0/catalog.dtd">'
    echo '<catalog xmlns="urn:oasis:names:tc:entity:xmlns:xml:catalog">'
    for v in $DBX_VERSIONS; do echo "  <nextCatalog catalog=\"file://$PREFIX/share/xml/docbook/xml-dtd-$v/catalog.xml\"/>"; done
    echo '</catalog>'
  } > "$STAGE$PREFIX/etc/xml/docbook-xml"
  # The root catalog: DocBook's identifiers delegated to the catalog above.
  {
    echo '<?xml version="1.0"?>'
    echo '<!DOCTYPE catalog PUBLIC "-//OASIS//DTD Entity Resolution XML Catalog V1.0//EN" "http://www.oasis-open.org/committees/entity/release/1.0/catalog.dtd">'
    echo '<catalog xmlns="urn:oasis:names:tc:entity:xmlns:xml:catalog">'
    local c="file://$PREFIX/etc/xml/docbook-xml" p
    for p in "-//OASIS//DTD DocBook" "-//OASIS//ELEMENTS DocBook" "-//OASIS//ENTITIES DocBook" \
             "-//OASIS//DTD XML Exchange Table Model" "ISO 8879:1986"; do
      echo "  <delegatePublic publicIdStartString=\"$p\" catalog=\"$c\"/>"
    done
    for p in "http://www.oasis-open.org/docbook/" "http://docbook.org/xml/" "https://docbook.org/xml/"; do
      echo "  <delegateSystem systemIdStartString=\"$p\" catalog=\"$c\"/>"
      echo "  <delegateURI uriStartString=\"$p\" catalog=\"$c\"/>"
    done
    echo '</catalog>'
  } > "$STAGE$PREFIX/etc/xml/catalog"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
