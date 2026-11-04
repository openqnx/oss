#!/bin/sh
# AsciiDoc.py: an AsciiDoc document with a title, sections, a list, a
# table, an attribute reference and a listing block converted to HTML5
# (known elements and text, attribute substituted, the listing escaped),
# to DocBook 4.5 (XML that xmllint parses, with the expected elements), and
# from stdin; a man page source converted with a2x -f manpage through
# xsltproc and the DocBook XSL stylesheets (catalog as in test_xmlto) to a
# roff page with the expected .TH line and NAME section.
echo "== asciidoc =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
T=$(mktemp -d /tmp/qnxport-asciidoc.XXXXXX) || { echo "FAIL asciidoc (mktemp)"; exit 1; }
trap 'cd /; rm -rf "$T"' EXIT
trap 'exit 1' INT TERM HUP
cd "$T" || exit 1

v=$(asciidoc --version 2>&1)
[ "$v" = "asciidoc 10.2.1" ] && ok "asciidoc --version: 10.2.1" || bad "version: $v"

cat > doc.txt <<'EOF'
QNX Ports Report
================
:board: QEMU x86_64

Introduction
------------
This report was rendered on {board}.

* first item
* second item

[options="header"]
|===
|Component |Version
|asciidoc  |10.2.1
|===

----
if (a < b && c > d) return;
----
EOF
asciidoc -b html5 -o doc.html doc.txt > html.log 2>&1; rc=$?
[ $rc -eq 0 ] && [ -s doc.html ] && ok "html5 backend: doc.html written" || bad "asciidoc html5 rc=$rc: $(head -c 300 html.log)"
grep -q '<title>QNX Ports Report</title>' doc.html && ok "HTML <title>" || bad "HTML title"
grep -q '<h2 id="_introduction">Introduction</h2>' doc.html && ok "section heading with its id" || bad "section heading: $(grep -m1 '<h2' doc.html)"
grep -q 'rendered on QEMU x86_64' doc.html && ok "attribute {board} substituted" || bad "attribute"
[ "$(grep -c '<li>' doc.html)" -eq 2 ] && ok "two list items" || bad "list items: $(grep -c '<li>' doc.html)"
grep -q '<th class="tableblock halign-left valign-top" >Component *</th>' doc.html && grep -q '<p class="tableblock">10.2.1</p>' doc.html && ok "table: header cell and body cell" || bad "table"
grep -q 'if (a &lt; b &amp;&amp; c &gt; d) return;' doc.html && ok "listing block HTML-escaped" || bad "listing"

asciidoc -b docbook45 -o doc.xml doc.txt > db.log 2>&1; rc=$?
[ $rc -eq 0 ] && grep -q '<article lang="en">' doc.xml && grep -q '<title>Introduction</title>' doc.xml && \
  grep -q '<itemizedlist>' doc.xml && ok "docbook45 backend: article, section title, itemizedlist" || bad "docbook45 rc=$rc: $(head -c 300 db.log)"
if command -v xmllint > /dev/null 2>&1; then
  xmllint --noout --nonet doc.xml > lint.log 2>&1 && ok "DocBook output is well-formed XML (xmllint)" || bad "xmllint: $(grep -v 'failed to load external entity\|warning' lint.log | head -3)"
fi
o=$(printf 'Hello *QNX*\n' | asciidoc -s -b html5 -o - - 2>&1)
case "$o" in *'<strong>QNX</strong>'*) ok "stdin to stdout, no header (-s): *QNX* -> <strong>" ;; *) bad "stdin: $o" ;; esac

X=
for d in /usr/local/share/xml/docbook/xsl-stylesheets-* /data/oss/usr/local/share/xml/docbook/xsl-stylesheets-*; do
  [ -f "$d/manpages/docbook.xsl" ] && { X=$d; break; }
done
cat > catalog.xml <<EOF
<?xml version="1.0"?>
<catalog xmlns="urn:oasis:names:tc:entity:xmlns:xml:catalog">
  <rewriteURI uriStartString="http://docbook.sourceforge.net/release/xsl/current/" rewritePrefix="file://$X/"/>
  <rewriteSystem systemIdStartString="http://docbook.sourceforge.net/release/xsl/current/" rewritePrefix="file://$X/"/>
  <nextCatalog catalog="file://$X/catalog.xml"/>
</catalog>
EOF
XML_CATALOG_FILES=$T/catalog.xml; export XML_CATALOG_FILES
cat > qnxtool.1.txt <<'EOF'
QNXTOOL(1)
==========
:doctype: manpage

NAME
----
qnxtool - demonstrate a2x on QNX

SYNOPSIS
--------
*qnxtool* ['OPTIONS']

DESCRIPTION
-----------
Prints nothing at all.
EOF
a2x -L -f manpage qnxtool.1.txt > a2x.log 2>&1; rc=$?
[ $rc -eq 0 ] && [ -f qnxtool.1 ] && ok "a2x -f manpage: qnxtool.1 written" || bad "a2x rc=$rc: $(tail -3 a2x.log)"
grep -q '^\.TH "QNXTOOL" "1"' qnxtool.1 2>/dev/null && ok "man page .TH \"QNXTOOL\" \"1\"" || bad ".TH: $(grep TH qnxtool.1 2>&1 | head -1)"
grep -q 'qnxtool \\- demonstrate a2x on QNX' qnxtool.1 2>/dev/null && ok "man page NAME section" || bad "NAME section"

cd /; rm -rf "$T"; trap - EXIT
[ $fail -eq 0 ] && { echo "PASS asciidoc"; exit 0; }
echo "FAIL asciidoc ($fail check(s) failed)"; exit 1
