#!/bin/sh
# xmllint and xmlcatalog: well-formedness errors are reported with a
# non-zero status; DTD validation (--valid) accepts a valid document and
# rejects an invalid one; XML Schema validation (--schema); --xpath with
# known results; --format pretty-prints; --c14n canonicalises attribute
# order; xmlcatalog creates a catalog, adds a rewriteURI entry and resolves
# a URI through it (also through xmllint, via XML_CATALOG_FILES).
echo "== libxml2-utils =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
T=$(mktemp -d /tmp/qnxport-xmllint.XXXXXX) || { echo "FAIL libxml2-utils (mktemp)"; exit 1; }
trap 'cd /; rm -rf "$T"' EXIT
trap 'exit 1' INT TERM HUP
cd "$T" || exit 1

v=$(xmllint --version 2>&1 | head -n 1)
echo "  info $v"
case "$v" in *"21305"*) ok "xmllint uses libxml2 2.13.5" ;; *) bad "version: $v" ;; esac

printf '<a><b></a>\n' > bad.xml
xmllint --noout bad.xml > bad.log 2>&1 && bad "malformed document accepted" || { grep -q 'Opening and ending tag mismatch' bad.log && ok "malformed document: tag mismatch reported" || bad "error text: $(head -2 bad.log)"; }

cat > valid.xml <<'XEOF'
<?xml version="1.0"?>
<!DOCTYPE ports [
  <!ELEMENT ports (port+)>
  <!ELEMENT port (#PCDATA)>
  <!ATTLIST port arch (x86_64|aarch64le) #REQUIRED>
]>
<ports><port arch="x86_64">cups</port><port arch="aarch64le">avahi</port></ports>
XEOF
sed 's/arch="aarch64le"/arch="mips"/' valid.xml > invalid.xml
xmllint --noout --valid valid.xml > v.log 2>&1 && ok "--valid: valid document accepted" || bad "--valid valid: $(head -3 v.log)"
xmllint --noout --valid invalid.xml > iv.log 2>&1 && bad "--valid accepted arch=mips" || ok "--valid: arch=mips rejected"

cat > s.xsd <<'XEOF'
<xs:schema xmlns:xs="http://www.w3.org/2001/XMLSchema">
  <xs:element name="n"><xs:simpleType><xs:restriction base="xs:integer"><xs:maxInclusive value="10"/></xs:restriction></xs:simpleType></xs:element>
</xs:schema>
XEOF
printf '<n>7</n>\n' > n7.xml; printf '<n>70</n>\n' > n70.xml
xmllint --noout --schema s.xsd n7.xml > /dev/null 2>&1 && ! xmllint --noout --schema s.xsd n70.xml > /dev/null 2>&1 \
  && ok "--schema: 7 valid, 70 above maxInclusive rejected" || bad "--schema"

o=$(xmllint --xpath 'count(//port)' valid.xml 2>&1)
[ "$o" = 2 ] && ok "--xpath count(//port) = 2" || bad "xpath count: $o"
o=$(xmllint --xpath 'string(//port[@arch="aarch64le"])' valid.xml 2>&1)
[ "$o" = avahi ] && ok "--xpath string(//port[@arch=aarch64le]) = avahi" || bad "xpath string: $o"

printf '<r><x>1</x></r>' > flat.xml
[ "$(xmllint --format flat.xml | sed -n 3p)" = "  <x>1</x>" ] && ok "--format indents children" || bad "--format: $(xmllint --format flat.xml)"
printf '<r b="2" a="1"/>' > c.xml
[ "$(xmllint --c14n c.xml)" = '<r a="1" b="2"></r>' ] && ok "--c14n: sorted attributes, explicit end tag" || bad "--c14n: $(xmllint --c14n c.xml)"

xmlcatalog --noout --create cat.xml && xmlcatalog --noout --add rewriteURI "http://example.org/qnx/" "file://$T/" cat.xml
o=$(xmlcatalog cat.xml http://example.org/qnx/flat.xml 2>/dev/null | tail -n 1)  # a SYSTEM lookup is tried first ("No entry for SYSTEM")
[ "$o" = "file://$T/flat.xml" ] && ok "xmlcatalog: rewriteURI added and resolved" || bad "xmlcatalog resolve: $o"
o=$(XML_CATALOG_FILES=$T/cat.xml xmllint --nonet --xpath 'string(/r/x)' http://example.org/qnx/flat.xml 2>&1)
[ "$o" = 1 ] && ok "xmllint --nonet reads http://example.org/qnx/flat.xml through the catalog" || bad "catalog in xmllint: $o"

cd /; rm -rf "$T"; trap - EXIT
[ $fail -eq 0 ] && { echo "PASS libxml2-utils"; exit 0; }
echo "FAIL libxml2-utils ($fail check(s) failed)"; exit 1
