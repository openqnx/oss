#!/bin/sh
# python3-lxml on the ported libxml2/libxslt: parse XML (namespaces,
# attributes, tail text), XPath with namespaces, functions and variables,
# an XSLT transform with a parameter and EXSLT, building and serialising a
# tree (pretty print, UTF-8, C14N), HTML parsing, XML Schema validation,
# lxml.objectify, iterparse, and a syntax error reported with its line.
echo "== python3-lxml =="
PY=${QNX_PORT_PYTHON:-/usr/local/bin/python3}; [ -x "$PY" ] || PY=python3
T=$(mktemp -d "${TMPDIR:-/tmp}/qnxport-lxml.XXXXXX") || { echo "FAIL python3-lxml (no temporary directory)"; exit 1; }
trap 'cd /; rm -rf "$T"' EXIT
cd "$T" || exit 1
cat > t.py <<'P'
import sys, io
fails = 0
def check(ok, what):
    global fails
    print(("  ok   " if ok else "  FAIL ") + what); fails += (not ok)
def info(s): print("  info " + s)
def run(name, fn):
    try: fn()
    except Exception as e:
        import traceback; traceback.print_exc(limit=3, file=sys.stdout)
        check(False, "%s: %r" % (name, e))
from lxml import etree
info("lxml %s, libxml2 %s, libxslt %s" % (".".join(map(str, etree.LXML_VERSION)),
     ".".join(map(str, etree.LIBXML_VERSION)), ".".join(map(str, etree.LIBXSLT_VERSION))))
DOC = b"""<?xml version="1.0" encoding="UTF-8"?>
<inv:inventory xmlns:inv="urn:qnx:inv" site="Ottawa">
  <inv:item sku="A1" qty="3"><inv:name>bolt</inv:name><inv:price>0.25</inv:price></inv:item>
  <inv:item sku="B2" qty="10"><inv:name>nut</inv:name><inv:price>0.10</inv:price></inv:item>
  <inv:item sku="C3" qty="1"><inv:name>gr\xc3\xbc\xc3\x9f</inv:name><inv:price>12.50</inv:price></inv:item>
  tail-text
</inv:inventory>"""
NS = {"i": "urn:qnx:inv"}
def parse():
    root = etree.fromstring(DOC)
    check(root.tag == "{urn:qnx:inv}inventory" and root.get("site") == "Ottawa", "root tag with namespace, attribute")
    items = root.findall("i:item", NS)
    check([i.get("sku") for i in items] == ["A1", "B2", "C3"], "findall with namespace map")
    check(items[2].findtext("i:name", namespaces=NS) == u"grüß", "UTF-8 text decoded")
    check(items[-1].tail.strip() == "tail-text", "tail text")
def xpath():
    root = etree.fromstring(DOC)
    total = root.xpath("sum(i:item/@qty)", namespaces=NS)
    check(total == 14.0, "XPath sum(@qty) = 14")
    value = root.xpath("sum(i:item/i:price)", namespaces=NS)
    check(abs(value - 12.85) < 1e-9, "XPath sum(price) = 12.85")
    names = root.xpath("i:item[@qty > $n]/i:name/text()", namespaces=NS, n=2)
    check(names == ["bolt", "nut"], "XPath with a variable: items with qty > 2")
    check(root.xpath("count(//i:item)", namespaces=NS) == 3 and
          root.xpath("string(i:item[last()]/@sku)", namespaces=NS) == "C3", "count(), last(), string()")
    f = etree.XPath("//i:item[i:name=$nm]/@sku", namespaces=NS)
    check(f(root, nm="nut") == ["B2"], "compiled XPath object")
def xslt():
    xsl = etree.XML(b"""<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
        xmlns:i="urn:qnx:inv" xmlns:math="http://exslt.org/math" extension-element-prefixes="math">
      <xsl:param name="min" select="0"/>
      <xsl:output method="xml" indent="no"/>
      <xsl:template match="/i:inventory">
        <report site="{@site}" max="{math:max(i:item/@qty)}">
          <xsl:for-each select="i:item[@qty &gt;= $min]">
            <xsl:sort select="@qty" data-type="number" order="descending"/>
            <line sku="{@sku}" value="{format-number(@qty * i:price, '0.00')}"/>
          </xsl:for-each>
        </report>
      </xsl:template>
    </xsl:stylesheet>""")
    tr = etree.XSLT(xsl)
    out = tr(etree.fromstring(DOC), min="2")
    r = out.getroot()
    check(r.tag == "report" and r.get("site") == "Ottawa", "XSLT output root and attribute value template")
    check(r.get("max") == "10", "EXSLT math:max")
    check([(l.get("sku"), l.get("value")) for l in r] == [("B2", "1.00"), ("A1", "0.75")],
          "XSLT parameter, sort, format-number")
def build():
    E = etree.Element("cfg", version="2")
    s = etree.SubElement(E, "server"); s.text = "qnx"
    etree.SubElement(E, "port").text = "8080"
    txt = etree.tostring(E, encoding="unicode")
    check(txt == '<cfg version="2"><server>qnx</server><port>8080</port></cfg>', "build and serialise a tree")
    pretty = etree.tostring(E, pretty_print=True).decode()
    check(pretty.count("\n") == 4, "pretty_print")
    c14n = etree.tostring(etree.fromstring('<a b="1"   a="2"><c/></a>'), method="c14n")
    check(c14n == b'<a a="2" b="1"><c></c></a>', "canonical XML (C14N)")
    x = etree.tostring(etree.fromstring(DOC), encoding="UTF-8", xml_declaration=True)
    check(x.startswith(b"<?xml version='1.0' encoding='UTF-8'?>") and u"grüß".encode() in x, "UTF-8 output with declaration")
def html():
    from lxml import html as H
    doc = H.fromstring("<html><body><p class=x>one<p>two<a href='/q'>link</a></body></html>")
    check([p.text_content() for p in doc.xpath("//p")] == ["one", "twolink"], "HTML parser closes unclosed <p>")
    check(doc.xpath("//a/@href") == ["/q"], "HTML attribute without quotes")
def schema():
    xsd = etree.XMLSchema(etree.XML(b"""<xs:schema xmlns:xs="http://www.w3.org/2001/XMLSchema">
      <xs:element name="pt"><xs:complexType>
        <xs:attribute name="x" type="xs:integer" use="required"/></xs:complexType></xs:element></xs:schema>"""))
    check(xsd.validate(etree.XML(b'<pt x="3"/>')), "XML Schema accepts a valid document")
    check(not xsd.validate(etree.XML(b'<pt x="three"/>')) and len(xsd.error_log) > 0, "XML Schema rejects an invalid one, with an error log")
def objectify():
    from lxml import objectify as O
    r = O.fromstring("<r><n>42</n><f>2.5</f><s>qnx</s></r>")
    check(r.n + 1 == 43 and abs(r.f * 2 - 5.0) < 1e-12 and r.s == "qnx", "objectify: typed child values")
def iterparse():
    n = [el.get("sku") for ev, el in etree.iterparse(io.BytesIO(DOC), tag="{urn:qnx:inv}item")]
    check(n == ["A1", "B2", "C3"], "iterparse with a tag filter")
def errors():
    try:
        etree.fromstring(b"<a>\n<b>\n</a>")
        check(False, "malformed XML raises XMLSyntaxError")
    except etree.XMLSyntaxError as e:
        check(e.lineno == 3, "malformed XML raises XMLSyntaxError at line 3")
for n, f in (("parse", parse), ("xpath", xpath), ("xslt", xslt), ("build", build), ("html", html),
             ("schema", schema), ("objectify", objectify), ("iterparse", iterparse), ("errors", errors)):
    run(n, f)
sys.exit(1 if fails else 0)
P
"$PY" t.py; rc=$?
[ $rc -eq 0 ] && { echo "PASS python3-lxml"; exit 0; }
echo "FAIL python3-lxml (rc=$rc)"; exit 1
