#!/usr/bin/env python3
"""Render an xdg-utils DocBook refentry (scripts/desc/*.xml) as plain text.

Stands in for "xmlto txt", which the xdg-utils build uses only to make the
text that each script prints for --help and --manual. xmlto needs the DocBook
XSL stylesheets and a text browser (w3m/lynx) on the build machine; this needs
only Python. The layout follows xmlto's: section headings at column 0 ("Name",
"Synopsis", "Description", ...), bodies indented by three spaces -- the
script generator (generate-help-script.awk) looks for the "Name" and
"Description" headings to cut out the usage text.

    refentry2txt.py desc/xdg-open.xml        -> ./xdg-open.txt
"""
import os
import sys
import textwrap
import xml.etree.ElementTree as ET

WIDTH = 72


def text(e):
    """Inline text of an element, with DocBook inline markup flattened."""
    out = [e.text or ""]
    for c in e:
        t = text(c)
        if c.tag == "quote":
            t = '"' + t + '"'
        out.append(t)
        out.append(c.tail or "")
    return "".join(out)


def norm(s):
    return " ".join(s.split())


def para(s, indent):
    s = norm(s)
    if not s:
        return []
    return textwrap.wrap(s, WIDTH, initial_indent=" " * indent,
                         subsequent_indent=" " * indent,
                         break_on_hyphens=False) + [""]


def synopsis_item(e):
    """cmdsynopsis pieces: arg/group with choice= and rep= as xmlto shows them."""
    if e.tag == "command":
        return norm(text(e))
    if e.tag in ("arg", "group"):
        parts = []
        if e.text and e.text.strip():
            parts.append(norm(e.text))
        for c in e:
            parts.append(synopsis_item(c) if c.tag in ("arg", "group") else norm(text(c)))
            if c.tail and c.tail.strip():
                parts.append(norm(c.tail))
        inner = (" | " if e.tag == "group" else " ").join(p for p in parts if p)
        choice = e.get("choice", "opt")
        if choice == "req":
            inner = "{" + inner + "}"
        elif choice == "opt":
            inner = "[" + inner + "]"
        if e.get("rep") == "repeat":
            inner += "..."
        return inner
    return norm(text(e))


def block(e, indent):
    """Lines for a block-level element."""
    tag = e.tag
    if tag in ("para", "simpara"):
        lines, buf = [], [e.text or ""]
        # A para may hold a nested block (a list, a listing); flush around it.
        for c in e:
            if c.tag in ("itemizedlist", "orderedlist", "variablelist",
                         "programlisting", "screen", "literallayout"):
                lines += para("".join(buf), indent)
                lines += block(c, indent)
                buf = [c.tail or ""]
            else:
                t = text(c)
                buf.append('"' + t + '"' if c.tag == "quote" else t)
                buf.append(c.tail or "")
        return lines + para("".join(buf), indent)
    if tag in ("programlisting", "screen", "literallayout"):
        body = text(e).strip("\n").splitlines()
        return [" " * (indent + 2) + l.rstrip() for l in body] + [""]
    if tag in ("itemizedlist", "orderedlist"):
        lines = []
        for n, li in enumerate(e.findall("listitem"), 1):
            mark = "* " if tag == "itemizedlist" else "%d. " % n
            sub = []
            for c in li:
                sub += block(c, indent + 2 + len(mark))
            if sub:
                first = sub[0].lstrip()
                sub[0] = " " * (indent + 2) + mark + first
            lines += sub
        return lines
    if tag == "variablelist":
        lines = []
        for ent in e.findall("varlistentry"):
            terms = ", ".join(norm(text(t)) for t in ent.findall("term"))
            lines.append(" " * indent + terms)
            lines.append("")
            for item in ent.findall("listitem"):
                for c in item:
                    lines += block(c, indent + 3)
        return lines
    if tag == "cmdsynopsis":
        return para(" ".join(synopsis_item(c) for c in e), indent)
    if tag in ("refsect2", "refsect3", "section"):
        lines = []
        t = e.find("title")
        if t is not None:
            lines += [" " * indent + norm(text(t)), ""]
        for c in e:
            if c.tag != "title":
                lines += block(c, indent + 2)
        return lines
    if tag == "title":
        return []
    # Anything else: treat its text as a paragraph.
    return para(text(e), indent)


def render(root):
    out = []
    nd = root.find("refnamediv")
    if nd is not None:
        names = ", ".join(norm(text(n)) for n in nd.findall("refname"))
        purpose = norm(text(nd.find("refpurpose"))) if nd.find("refpurpose") is not None else ""
        out += ["Name", ""] + para(names + (" - " + purpose if purpose else ""), 3)
    syn = root.find("refsynopsisdiv")
    if syn is not None:
        out += ["Synopsis", ""]
        for c in syn:
            out += block(c, 3)
    for sect in root.findall("refsect1"):
        t = sect.find("title")
        out += [norm(text(t)) if t is not None else "", ""]
        for c in sect:
            out += block(c, 3)
    while out and out[-1] == "":
        out.pop()
    return "\n".join(out) + "\n"


def main():
    src = sys.argv[-1]
    # The desc/*.xml files have a DOCTYPE with no internal entities that
    # matter here; ElementTree does not fetch the external DTD.
    root = ET.parse(src).getroot()
    dst = os.path.splitext(os.path.basename(src))[0] + ".txt"
    with open(dst, "w", encoding="utf-8") as f:
        f.write(render(root))


if __name__ == "__main__":
    main()
