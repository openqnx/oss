#!/usr/bin/env python3
"""mdlite -- the small Markdown subset the guides use (headings, tables, fenced
code, lists, block quotes, rules, **bold**, `code`, links) as HTML. Every code
block gets a copy button whose text has the "host$ " / "qnx# " prompts
removed (COPY_CSS, COPY_JS). Used by scripts/gen-pkg-site for docs/guide/."""
import html, re

E = html.escape


def inline(t):
    out, parts = [], re.split(r"(`[^`]+`)", t)
    for p in parts:
        if p.startswith("`") and p.endswith("`") and len(p) > 1:
            out.append("<code>" + E(p[1:-1]) + "</code>")
            continue
        p = E(p)
        p = re.sub(r"\*\*(.+?)\*\*", r"<b>\1</b>", p)
        p = re.sub(r"\[([^\]]+)\]\(([^)]+)\)", r'<a href="\2">\1</a>', p)
        p = re.sub(r'(?<!href=")(https?://[^\s<)]+)', r'<a href="\1">\1</a>', p)
        out.append(p)
    return "".join(out)


def table(rows):
    cells = [[c.strip() for c in r.strip().strip("|").split("|")] for r in rows]
    head, body = cells[0], [r for r in cells[1:] if not re.fullmatch(r"[-: ]*", "".join(r))]
    h = "<div class=tw><table><thead><tr>" + "".join("<th>%s</th>" % inline(c) for c in head) + "</tr></thead><tbody>"
    for r in body:
        h += "<tr>" + "".join("<td>%s</td>" % inline(c) for c in r) + "</tr>"
    return h + "</tbody></table></div>"


def render(text):
    """Markdown text -> HTML body."""
    lines = text.split("\n")
    out, i, n_code = [], 0, 0
    while i < len(lines):
        l = lines[i]
        s = l.strip()
        if s.startswith("```"):
            j = i + 1
            while j < len(lines) and not lines[j].strip().startswith("```"):
                j += 1
            code = lines[i + 1:j]
            copy = "\n".join(re.sub(r"^(host\$|qnx#) ?", "", c) for c in code)
            n_code += 1
            out.append('<div class=code><button class=cp data-i="%d">Copy</button>'
                       '<pre id="c%d">%s</pre><textarea hidden id="t%d">%s</textarea></div>'
                       % (n_code, n_code, E("\n".join(code)), n_code, E(copy)))
            i = j + 1
            continue
        if s.startswith("|"):
            j = i
            while j < len(lines) and lines[j].strip().startswith("|"):
                j += 1
            out.append(table(lines[i:j]))
            i = j
            continue
        m = re.match(r"(#{1,4}) (.*)", l)
        if m:
            k = len(m.group(1))
            out.append("<h%d>%s</h%d>" % (k, inline(m.group(2)), k))
            i += 1
            continue
        if s == "---":
            out.append("<hr>")
            i += 1
            continue
        if s.startswith(">"):
            q = []
            while i < len(lines) and lines[i].strip().startswith(">"):
                q.append(lines[i].strip()[1:].strip())
                i += 1
            out.append("<blockquote>" + inline(" ".join(q)) + "</blockquote>")
            continue
        if re.match(r"\s*- ", l):
            items = []
            while i < len(lines) and (re.match(r"\s*- ", lines[i]) or (lines[i].startswith("  ") and lines[i].strip())):
                if re.match(r"\s*- ", lines[i]):
                    items.append([re.sub(r"^\s*- ", "", lines[i])])
                elif lines[i].strip().startswith("|"):
                    j = i
                    while j < len(lines) and lines[j].strip().startswith("|"):
                        j += 1
                    items[-1].append(("T", lines[i:j]))
                    i = j
                    continue
                else:
                    items[-1].append(lines[i].strip())
                i += 1
            h = "<ul>"
            for it in items:
                txt = " ".join(x for x in it if isinstance(x, str))
                tb = "".join(table(x[1]) for x in it if isinstance(x, tuple))
                h += "<li>" + inline(txt) + tb + "</li>"
            out.append(h + "</ul>")
            continue
        if not s:
            i += 1
            continue
        p = []
        while i < len(lines) and lines[i].strip() and not re.match(r"(#{1,4} |\||```|>|\s*- |---$)", lines[i].strip()):
            p.append(lines[i].strip())
            i += 1
        out.append("<p>" + inline(" ".join(p)) + "</p>")


    return "\n".join(out)


COPY_CSS = """.code{position:relative;margin:12px 0}.code pre{margin:0}
.cp{position:absolute;top:6px;right:6px;font:12px system-ui;padding:3px 9px;border:1px solid var(--line,#ccc);border-radius:6px;background:var(--bg,#fff);color:inherit;cursor:pointer}
.cp.ok{border-color:#2e9b4f;color:#2e9b4f}"""
COPY_JS = """<script>document.querySelectorAll('.cp').forEach(function(b){b.addEventListener('click',function(){
var t=document.getElementById('t'+b.dataset.i).value;function d(){b.textContent='Copied';b.classList.add('ok');setTimeout(function(){b.textContent='Copy';b.classList.remove('ok')},1500)}
if(navigator.clipboard&&window.isSecureContext){navigator.clipboard.writeText(t).then(d)}else{var a=document.createElement('textarea');a.value=t;document.body.appendChild(a);a.select();document.execCommand('copy');document.body.removeChild(a);d()}})});</script>"""
