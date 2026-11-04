#!/bin/sh
# docbook-xml: the DTDs through the XML catalogs, as libxml2's tools use them
# offline (--nonet). Known answers: xmlcatalog resolves the DocBook public
# and system identifiers of 4.5, 4.2 and 4.1.2 to the installed files;
# xmllint --valid accepts DocBook 4.5 and 4.2 documents through the default
# root catalog (no XML_CATALOG_FILES), expands a DocBook character entity,
# and rejects a document that breaks the content model.
echo "== docbook-xml =="
P=${QNX_PORT_PREFIX:-/usr/local}
CAT=$P/etc/xml/catalog
D=file://${QNX_PORT_DTD_PREFIX:-/usr/local}/share/xml/docbook
fails=0
ok()  { echo "  ok   $1"; }
bad() { echo "  FAIL $1"; fails=$((fails + 1)); }
chk() { if [ "$2" = "$3" ]; then ok "$1"; else bad "$1"; echo "  info want: $3"; echo "  info got:  $2"; fi; }
XC=$P/bin/xmlcatalog; XL=$P/bin/xmllint
[ -x "$XC" ] && [ -x "$XL" ] && ok "xmlcatalog and xmllint installed" || { bad "xmlcatalog and xmllint ($P/bin)"; echo "FAIL docbook-xml"; exit 1; }
[ -f "$CAT" ] && ok "root catalog $CAT" || bad "root catalog $CAT"

chk "public -//OASIS//DTD DocBook XML V4.5//EN" "$("$XC" "$CAT" "-//OASIS//DTD DocBook XML V4.5//EN" 2>&1)" "$D/xml-dtd-4.5/docbookx.dtd"
chk "public -//OASIS//ENTITIES DocBook Character Entities V4.5//EN" "$("$XC" "$CAT" "-//OASIS//ENTITIES DocBook Character Entities V4.5//EN" 2>&1)" "$D/xml-dtd-4.5/dbcentx.mod"
chk "public -//OASIS//DTD DocBook XML V4.2//EN" "$("$XC" "$CAT" "-//OASIS//DTD DocBook XML V4.2//EN" 2>&1)" "$D/xml-dtd-4.2/docbookx.dtd"
chk "public -//OASIS//DTD DocBook XML V4.1.2//EN" "$("$XC" "$CAT" "-//OASIS//DTD DocBook XML V4.1.2//EN" 2>&1)" "$D/xml-dtd-4.1.2/docbookx.dtd"
chk "system http://www.oasis-open.org/docbook/xml/4.3/docbookx.dtd" "$("$XC" "$CAT" "http://www.oasis-open.org/docbook/xml/4.3/docbookx.dtd" 2>&1)" "$D/xml-dtd-4.3/docbookx.dtd"
chk "system http://www.oasis-open.org/docbook/xml/4.4/ent/isonum.ent" "$("$XC" "$CAT" "http://www.oasis-open.org/docbook/xml/4.4/ent/isonum.ent" 2>&1)" "$D/xml-dtd-4.4/ent/isonum.ent"
out=$("$XC" "$CAT" "-//OASIS//DTD DocBook XML V9.9//EN" 2>&1); rc=$?
[ $rc -ne 0 ] && ok "an unknown version does not resolve (rc $rc)" || { bad "an unknown version does not resolve"; echo "  info $out"; }

T=${TMPDIR:-/tmp}/docbook-xml-test.$$; mkdir -p "$T"
cat > "$T/ok45.xml" <<'X'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE article PUBLIC "-//OASIS//DTD DocBook XML V4.5//EN" "http://www.oasis-open.org/docbook/xml/4.5/docbookx.dtd">
<article id="t">
  <title>QNX test</title>
  <section><title>One</title>
    <para>A list&mdash;of two:</para>
    <itemizedlist><listitem><para>first</para></listitem><listitem><para>second</para></listitem></itemizedlist>
    <informaltable><tgroup cols="2"><tbody><row><entry>a</entry><entry>b</entry></row></tbody></tgroup></informaltable>
  </section>
</article>
X
cat > "$T/ok42.xml" <<'X'
<?xml version="1.0"?>
<!DOCTYPE book PUBLIC "-//OASIS//DTD DocBook XML V4.2//EN" "http://www.oasis-open.org/docbook/xml/4.2/docbookx.dtd">
<book><title>B</title><chapter><title>C</title><para>Text &copy; 2026.</para></chapter></book>
X
cat > "$T/bad45.xml" <<'X'
<?xml version="1.0"?>
<!DOCTYPE article PUBLIC "-//OASIS//DTD DocBook XML V4.5//EN" "http://www.oasis-open.org/docbook/xml/4.5/docbookx.dtd">
<article><title>T</title><para><section><title>no</title></section></para><bogus/></article>
X
unset XML_CATALOG_FILES
# Without a catalog for the root, xmllint would try the network: --nonet
# makes a missing mapping fail instead.
for f in ok45 ok42; do
  out=$("$XL" --nonet --noout --valid "$T/$f.xml" 2>&1); rc=$?
  [ $rc -eq 0 ] && [ -z "$out" ] && ok "xmllint --valid --nonet $f.xml (default catalog)" || { bad "xmllint --valid --nonet $f.xml (rc $rc)"; echo "$out" | head -5; }
done
out=$("$XL" --nonet --noent --valid --xpath 'string(//section/para[1])' "$T/ok45.xml" 2>&1)
chk "&mdash; expands to U+2014 (DocBook entity sets)" "$out" "A list—of two:"
out=$("$XL" --nonet --noout --valid "$T/bad45.xml" 2>&1); rc=$?
if [ $rc -ne 0 ] && echo "$out" | grep -q "validity error" && echo "$out" | grep -q "bogus"; then
  ok "an invalid document is rejected (validity errors, rc $rc)"
else bad "an invalid document is rejected (rc $rc)"; echo "$out" | head -5; fi
rm -rf "$T"
[ $fails -eq 0 ] && { echo "PASS docbook-xml"; exit 0; }
echo "FAIL docbook-xml ($fails)"; exit 1
