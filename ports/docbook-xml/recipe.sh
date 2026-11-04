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
