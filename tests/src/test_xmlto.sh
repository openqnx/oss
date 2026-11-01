#!/bin/sh
# xmlto: a DocBook refentry converted to a man page (known .TH line, NAME
# and OPTIONS sections) and to a single XHTML page (title, the option as a
# term), through xsltproc and the DocBook XSL stylesheets; xmlto's own
# DocBook profiling (--stringparam profile.condition=arm) keeps only the
# matching paragraph (--profile); xmlif, its preprocessor, resolves if/elif/else on
# condition=pdf the way its upstream test does; an unknown format is
# refused. The stylesheets are found through an XML catalog: the
# docbook-xsl component's own catalog plus the docbook.sourceforge.net URL
# xmlto names (mapped to the same directory).
echo "== xmlto =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
T=$(mktemp -d /tmp/qnxport-xmlto.XXXXXX) || { echo "FAIL xmlto (mktemp)"; exit 1; }
trap 'cd /; rm -rf "$T"' EXIT
trap 'exit 1' INT TERM HUP
cd "$T" || exit 1

X=
for d in /usr/local/share/xml/docbook/xsl-stylesheets-* /data/oss/usr/local/share/xml/docbook/xsl-stylesheets-*; do
  [ -f "$d/manpages/docbook.xsl" ] && { X=$d; break; }
done
[ -n "$X" ] && ok "DocBook XSL stylesheets at $X" || bad "DocBook XSL stylesheets not found"
cat > catalog.xml <<EOF
<?xml version="1.0"?>
<catalog xmlns="urn:oasis:names:tc:entity:xmlns:xml:catalog">
  <rewriteURI uriStartString="http://docbook.sourceforge.net/release/xsl/current/" rewritePrefix="file://$X/"/>
  <rewriteSystem systemIdStartString="http://docbook.sourceforge.net/release/xsl/current/" rewritePrefix="file://$X/"/>
  <nextCatalog catalog="file://$X/catalog.xml"/>
</catalog>
EOF
XML_CATALOG_FILES=$T/catalog.xml; export XML_CATALOG_FILES

v=$(xmlto --version 2>&1)
[ "$v" = "xmlto version 0.0.29" ] && ok "xmlto --version: 0.0.29" || bad "version: $v"

# One line, no indentation: the DocBook XSL 1.79.2 manpages stylesheet
# carries the source's indentation into the roff output (" .SH ..."), on
# Linux as well -- not what this test is about.
cat > qnxdemo.xml <<'EOF'
<?xml version="1.0"?>
<refentry id="qnxdemo"><refmeta><refentrytitle>qnxdemo</refentrytitle><manvolnum>8</manvolnum></refmeta><refnamediv><refname>qnxdemo</refname><refpurpose>show xmlto working on QNX</refpurpose></refnamediv><refsynopsisdiv><cmdsynopsis><command>qnxdemo</command><arg>-v</arg></cmdsynopsis></refsynopsisdiv><refsect1><title>Description</title><para>Converted by xmlto on QNX.</para><para condition="x86">Only on x86_64.</para><para condition="arm">Only on aarch64.</para></refsect1><refsect1><title>Options</title><variablelist><varlistentry><term><option>-v</option></term><listitem><para>Be verbose.</para></listitem></varlistentry></variablelist></refsect1></refentry>
EOF

mkdir man
xmlto --skip-validation -o man man qnxdemo.xml > man.log 2>&1; rc=$?
[ $rc -eq 0 ] && [ -f man/qnxdemo.8 ] && ok "xmlto man: man/qnxdemo.8 written" || bad "xmlto man rc=$rc: $(head -c 400 man.log)"
grep -q '^\.TH "QNXDEMO" "8"' man/qnxdemo.8 2>/dev/null && ok ".TH \"QNXDEMO\" \"8\"" || bad ".TH: $(grep '^\.TH' man/qnxdemo.8 2>&1)"
grep -q 'qnxdemo \\- show xmlto working on QNX' man/qnxdemo.8 2>/dev/null && ok "NAME: qnxdemo \\- show xmlto working on QNX" || bad "NAME section"
grep -q '^\.SH "OPTIONS"' man/qnxdemo.8 2>/dev/null && grep -q 'Be verbose' man/qnxdemo.8 && ok "OPTIONS section with -v" || bad "OPTIONS section"
grep -q 'Only on x86_64' man/qnxdemo.8 && grep -q 'Only on aarch64' man/qnxdemo.8 && ok "unprofiled: both conditional paragraphs" || bad "unprofiled output"

# xmlto --profile runs DocBook XSL's profiling/profile.xsl first.
if [ -f "$X/profiling/profile.xsl" ]; then
  mkdir prof
  xmlto --skip-validation --profile --stringparam profile.condition=arm -o prof man qnxdemo.xml > prof.log 2>&1
  grep -q 'Only on aarch64' prof/qnxdemo.8 2>/dev/null && ! grep -q 'Only on x86_64' prof/qnxdemo.8 \
    && ok "profiling (profile.condition=arm): only the aarch64 paragraph" || bad "profiling: $(head -c 300 prof.log)"
else
  echo "  info profiling not checked: the installed DocBook XSL has no profiling/ stylesheets"
fi

mkdir html
xmlto --skip-validation -o html xhtml-nochunks qnxdemo.xml > html.log 2>&1; rc=$?
H=$(ls html/*.html 2>/dev/null | head -n 1)
[ $rc -eq 0 ] && [ -n "$H" ] && ok "xmlto xhtml-nochunks: $(basename "$H")" || bad "xhtml rc=$rc: $(head -c 400 html.log)"
grep -q '<title>qnxdemo</title>' "$H" 2>/dev/null && grep -q 'Be verbose' "$H" && ok "XHTML: <title>qnxdemo</title>, option text" || bad "XHTML content"

xmlto --skip-validation -o html no-such-format qnxdemo.xml > bad.log 2>&1 && bad "unknown format accepted" || ok "unknown format refused"

cat > if.xml <<'EOF'
Always.
<?xmlif if condition='html'?>
HTML only.
<?xmlif elif condition='pdf|ps'?>
PDF or PS.
<?xmlif else?>
Anything else.
<?xmlif fi?>
End.
EOF
o=$(xmlif condition=pdf < if.xml | grep -v '^$' | tr '\n' '|')
[ "$o" = "Always.|PDF or PS.|End.|" ] && ok "xmlif condition=pdf: elif branch" || bad "xmlif pdf: $o"
o=$(xmlif condition=foo < if.xml | grep -v '^$' | tr '\n' '|')
[ "$o" = "Always.|Anything else.|End.|" ] && ok "xmlif condition=foo: else branch" || bad "xmlif foo: $o"

cd /; rm -rf "$T"; trap - EXIT
[ $fail -eq 0 ] && { echo "PASS xmlto"; exit 0; }
echo "FAIL xmlto ($fail check(s) failed)"; exit 1
