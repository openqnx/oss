#!/bin/sh
# Pure-Python modules, each with a known answer, on the ported CPython:
# tornado (an HTTP server and client on the loopback), curio + sniffio
# (tasks, and which async library runs them), pytest + pytest-cov (a test
# run measured), click, idna, sortedcontainers, astor, traitlets, pygments,
# pytokens, coverage, async_generator, pretend, mypy_extensions, versioneer.
echo "== pymods =="
PY=/usr/local/bin/python3; [ -x "$PY" ] || PY=python3
T=$(mktemp -d /tmp/pymods.XXXXXX); cd "$T" || exit 1
cat > t.py <<'P'
import sys
fails = 0
def check(ok, what):
    global fails
    print(("  ok   " if ok else "  FAIL ") + what); fails += (not ok)
def run(name, fn):
    try: fn()
    except Exception as e: check(False, "%s: %r" % (name, e))

def t_tornado():
    import tornado.web, tornado.httpclient, tornado.ioloop, asyncio
    class H(tornado.web.RequestHandler):
        def get(self, who): self.write({"hello": who})
    async def main():
        app = tornado.web.Application([(r"/hi/(\w+)", H)])
        srv = app.listen(0, address="127.0.0.1")
        port = list(srv._sockets.values())[0].getsockname()[1]
        r = await tornado.httpclient.AsyncHTTPClient().fetch("http://127.0.0.1:%d/hi/qnx" % port)
        srv.stop(); return r.code, r.body
    code, body = asyncio.run(main())
    check(code == 200 and body == b'{"hello": "qnx"}', "tornado: GET /hi/qnx -> 200 %s" % body.decode())
def t_curio():
    import curio, sniffio
    async def child(n): await curio.sleep(0.01); return n * n
    async def main():
        ts = [await curio.spawn(child, i) for i in range(5)]
        return [await t.join() for t in ts], sniffio.current_async_library()
    r, lib = curio.run(main)
    check(r == [0, 1, 4, 9, 16], "curio: 5 tasks joined, results in order")
    check(lib == "curio", "sniffio: detects curio")
def t_pytest():
    open("test_demo.py", "w").write("def add(a, b):\n    return a + b\n\ndef test_add():\n    assert add(2, 3) == 5\n")
    import subprocess
    p = subprocess.run([sys.executable, "-m", "pytest", "-q", "-p", "no:cacheprovider", "--cov=.", "--cov-report=term", "test_demo.py"],
                       capture_output=True, text=True)
    out = p.stdout
    check(p.returncode == 0 and "1 passed" in out, "pytest: 1 passed")
    check("test_demo.py" in out and "100%" in out, "pytest-cov: coverage report, 100%")
def t_misc():
    import click
    from click.testing import CliRunner
    @click.command()
    @click.option("--n", type=int, default=1)
    def hello(n): click.echo("hi " * n)
    check(CliRunner().invoke(hello, ["--n", "2"]).output == "hi hi \n", "click: command with an option")
    import idna
    check(idna.encode("münchen.de") == b"xn--mnchen-3ya.de", "idna: münchen.de -> xn--mnchen-3ya.de")
    from sortedcontainers import SortedList
    s = SortedList([5, 1, 3]); s.add(2)
    check(list(s) == [1, 2, 3, 5] and s.bisect_left(3) == 2, "sortedcontainers: sorted, bisect")
    import astor, ast
    check(astor.to_source(ast.parse("x=(1+2)*3")).strip() == "x = (1 + 2) * 3", "astor: AST back to source")
    import traitlets
    class C(traitlets.HasTraits):
        n = traitlets.Int(0)
    seen = []; c = C(); c.observe(lambda ch: seen.append(ch["new"]), "n"); c.n = 7
    check(seen == [7], "traitlets: observed change")
    try: c.n = "x"; check(False, "traitlets: type checked")
    except traitlets.TraitError: check(True, "traitlets: type checked")
    import pygments, pygments.lexers, pygments.formatters
    h = pygments.highlight("def f(): pass", pygments.lexers.PythonLexer(), pygments.formatters.HtmlFormatter())
    check('<span class="k">def</span>' in h, "pygments: Python highlighted to HTML")
    import pytokens
    toks = [t.type.name for t in pytokens.tokenize("a = 1\n")]
    check("identifier" in toks and "number" in toks, "pytokens: identifier, number")
    import coverage
    cv = coverage.Coverage(data_file=None); cv.start()
    def sq(x): return x * x
    sq(3); cv.stop()
    check(any(f.endswith("t.py") for f in cv.get_data().measured_files()), "coverage: this file measured")
    from async_generator import async_generator, yield_
    import asyncio
    @async_generator
    async def gen():
        for i in range(3): await yield_(i)
    async def coll(): return [x async for x in gen()]
    check(asyncio.run(coll()) == [0, 1, 2], "async_generator: async for over it")
    import pretend
    stub = pretend.stub(name="qnx", f=pretend.call_recorder(lambda x: x + 1))
    check(stub.f(1) == 2 and stub.f.calls == [pretend.call(1)] and stub.name == "qnx", "pretend: stub and call recorder")
    import mypy_extensions
    check(hasattr(mypy_extensions, "trait") and hasattr(mypy_extensions, "mypyc_attr"), "mypy_extensions: trait, mypyc_attr")
    import versioneer
    check(hasattr(versioneer, "get_versions"), "versioneer: get_versions")

for n, f in (("tornado", t_tornado), ("curio", t_curio), ("pytest", t_pytest), ("misc", t_misc)):
    run(n, f)
print(("PASS" if fails == 0 else "FAIL") + " pymods" + ("" if fails == 0 else " (%d check(s) failed)" % fails))
sys.exit(1 if fails else 0)
P
"$PY" t.py; rc=$?
cd /; rm -rf "$T"; exit $rc
