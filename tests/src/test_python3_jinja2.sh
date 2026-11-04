#!/bin/sh
# python3-jinja2 and python3-markupsafe: a template with inheritance, a
# loop, filters, a macro and autoescaping renders the exact expected text;
# undefined variables raise under StrictUndefined; the sandbox refuses
# unsafe attribute access. MarkupSafe escapes with its C speedups
# (markupsafe._speedups) and leaves Markup strings alone.
echo "== python3-jinja2 =="
PY=${QNX_PORT_PYTHON:-/usr/local/bin/python3}; [ -x "$PY" ] || PY=python3
# No __pycache__ left in the installation's site-packages (docs/porting-hazards.md 183)
export PYTHONDONTWRITEBYTECODE=1
T=$(mktemp -d "${TMPDIR:-/tmp}/qnxport-jinja2.XXXXXX") || { echo "FAIL python3-jinja2 (no temporary directory)"; exit 1; }
trap 'cd /; rm -rf "$T"' EXIT
cd "$T" || exit 1
cat > t.py <<'P'
import sys
fails = 0
def check(ok, what):
    global fails
    print(("  ok   " if ok else "  FAIL ") + what); fails += (not ok)
def info(s): print("  info " + s)
import jinja2, markupsafe
from markupsafe import Markup, escape
info("Jinja2 %s, MarkupSafe %s" % (jinja2.__version__, markupsafe.__version__))
import markupsafe._speedups as sp
check(sp.__file__.endswith(".so"), "markupsafe._speedups is the compiled extension")
check(markupsafe._escape_inner is sp._escape_inner, "escape() uses the C speedups")
check(str(escape('<a href="x">&\'</a>')) == "&lt;a href=&#34;x&#34;&gt;&amp;&#39;&lt;/a&gt;", "escape() of HTML special characters")
check(escape(Markup("<b>ok</b>")) == "<b>ok</b>" and Markup("<i>%s</i>") % "<x>" == "<i>&lt;x&gt;</i>", "Markup is left alone, formatting escapes")
check(Markup("<p>a &amp; b</p>").striptags() == "a & b", "striptags() unescapes")
loader = jinja2.DictLoader({
    "base.html": "<title>{% block title %}{% endblock %}</title>\n{% block body %}{% endblock %}",
    "page.html": "{% extends 'base.html' %}{% from 'm.html' import item %}"
                 "{% block title %}{{ title|upper }}{% endblock %}"
                 "{% block body %}<ul>{% for p in procs|sort(attribute='pid') %}{{ item(p) }}{% endfor %}</ul>"
                 "\n{{ procs|map(attribute='mem')|sum }} KiB, {{ procs|length }} procs{% endblock %}",
    "m.html": "{% macro item(p) %}<li class=\"{{ loop_class|default('p') }}\">{{ p.pid }}:{{ p.name }}</li>{% endmacro %}",
})
env = jinja2.Environment(loader=loader, autoescape=True)
out = env.get_template("page.html").render(title="pidin", procs=[
    {"pid": 7, "name": "devc-<pty>", "mem": 300}, {"pid": 1, "name": "procnto", "mem": 1024}])
want = ('<title>PIDIN</title>\n<ul><li class="p">1:procnto</li><li class="p">7:devc-&lt;pty&gt;</li></ul>'
        "\n1324 KiB, 2 procs")
check(out == want, "inheritance, macro, sort/map/sum filters and autoescape render the expected page")
if out != want: info(repr(out))
t = jinja2.Environment().from_string("{% set xs = range(1, 6) %}{{ xs|select('odd')|join(',') }} {{ '%05.1f'|format(3.14159) }} {{ 'a b c'.split()|reverse|join }}")
check(t.render() == "1,3,5 003.1 cba", "select/join/format filters and method calls")
try:
    jinja2.Environment(undefined=jinja2.StrictUndefined).from_string("{{ nope }}").render()
    check(False, "StrictUndefined raises UndefinedError")
except jinja2.UndefinedError:
    check(True, "StrictUndefined raises UndefinedError")
try:
    jinja2.Environment().from_string("{% for %}")
    check(False, "syntax error raises TemplateSyntaxError")
except jinja2.TemplateSyntaxError as e:
    check(e.lineno == 1, "syntax error raises TemplateSyntaxError at line 1")
from jinja2.sandbox import SandboxedEnvironment, SecurityError
try:
    SandboxedEnvironment().from_string("{{ ''.__class__.__mro__ }}").render()
    check(False, "sandbox refuses __class__")
except SecurityError:
    check(True, "sandbox refuses __class__")
check(env.from_string("{{ x }}").render(x="<s>") == "&lt;s&gt;", "autoescape of a plain variable")
sys.exit(1 if fails else 0)
P
"$PY" t.py; rc=$?
[ $rc -eq 0 ] && { echo "PASS python3-jinja2"; exit 0; }
echo "FAIL python3-jinja2 (rc=$rc)"; exit 1
