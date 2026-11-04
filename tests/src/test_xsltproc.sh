#!/bin/sh
# xsltproc: an XSLT 1.0 stylesheet applied to a small inventory -- sorting,
# xsl:key grouping, sum(), a --stringparam, xsl:output text -- with a known
# text result; EXSLT (str:tokenize, math:max) through libexslt; -o writes
# the result to a file; --xinclude merges an included document; a
# malformed stylesheet is refused with a non-zero status.
echo "== xsltproc =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
T=$(mktemp -d /tmp/qnxport-xsltproc.XXXXXX) || { echo "FAIL xsltproc (mktemp)"; exit 1; }
trap 'cd /; rm -rf "$T"' EXIT
trap 'exit 1' INT TERM HUP
cd "$T" || exit 1

v=$(xsltproc --version 2>&1 | head -n 1)
echo "  info $v"
case "$v" in *"libxslt 10142"*) ok "xsltproc uses libxslt 1.1.42" ;; *) bad "version: $v" ;; esac

cat > in.xml <<'XEOF'
<inventory>
  <item arch="aarch64le" name="zstd" size="700"/>
  <item arch="x86_64" name="cups" size="1100"/>
  <item arch="aarch64le" name="avahi" size="400"/>
  <item arch="x86_64" name="7zip" size="3300"/>
</inventory>
XEOF
cat > report.xsl <<'XEOF'
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="text"/>
  <xsl:param name="title" select="'none'"/>
  <xsl:key name="by-arch" match="item" use="@arch"/>
  <xsl:template match="/">
    <xsl:value-of select="concat('[', $title, ']&#10;')"/>
    <xsl:for-each select="//item[generate-id() = generate-id(key('by-arch', @arch)[1])]">
      <xsl:sort select="@arch"/>
      <xsl:value-of select="concat(@arch, ': ', count(key('by-arch', @arch)), ' items, ', sum(key('by-arch', @arch)/@size), ' KB&#10;')"/>
      <xsl:for-each select="key('by-arch', @arch)">
        <xsl:sort select="@name"/>
        <xsl:value-of select="concat('  ', @name, '&#10;')"/>
      </xsl:for-each>
    </xsl:for-each>
  </xsl:template>
</xsl:stylesheet>
XEOF
xsltproc --stringparam title "QNX 8.0" report.xsl in.xml > out.txt 2> err.txt; rc=$?
printf '[QNX 8.0]\naarch64le: 2 items, 1100 KB\n  avahi\n  zstd\nx86_64: 2 items, 4400 KB\n  7zip\n  cups\n' > expect.txt
[ $rc -eq 0 ] && cmp -s out.txt expect.txt && ok "grouping, sorting, sum and --stringparam: known result" || { bad "report rc=$rc: $(cat err.txt)"; sed 's/^/       /' out.txt; }

cat > exslt.xsl <<'XEOF'
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings" xmlns:math="http://exslt.org/math" extension-element-prefixes="str math">
  <xsl:output method="text"/>
  <xsl:template match="/">
    <xsl:for-each select="str:tokenize('b,a,c', ',')"><xsl:sort/><xsl:value-of select="."/></xsl:for-each>
    <xsl:value-of select="concat(' ', math:max(//item/@size))"/>
  </xsl:template>
</xsl:stylesheet>
XEOF
o=$(xsltproc exslt.xsl in.xml 2>&1)
[ "$o" = "abc 3300" ] && ok "EXSLT str:tokenize and math:max" || bad "EXSLT: $o"

xsltproc -o result.txt --stringparam title x report.xsl in.xml 2>/dev/null && head -n 1 result.txt | grep -q '^\[x\]$' && ok "-o writes the result file" || bad "-o"

printf '<part>included text</part>\n' > part.xml
printf '<doc xmlns:xi="http://www.w3.org/2001/XInclude"><xi:include href="part.xml"/></doc>\n' > main.xml
printf '<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"><xsl:output method="text"/><xsl:template match="/"><xsl:value-of select="/doc/part"/></xsl:template></xsl:stylesheet>\n' > x.xsl
o=$(xsltproc --xinclude x.xsl main.xml 2>&1)
[ "$o" = "included text" ] && ok "--xinclude merges the included document" || bad "xinclude: $o"

printf '<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"><xsl:template match="/"><xsl:value-of/></xsl:template>' > broken.xsl
xsltproc broken.xsl in.xml > /dev/null 2>&1 && bad "malformed stylesheet accepted" || ok "malformed stylesheet refused"

cd /; rm -rf "$T"; trap - EXIT
[ $fail -eq 0 ] && { echo "PASS xsltproc"; exit 0; }
echo "FAIL xsltproc ($fail check(s) failed)"; exit 1
