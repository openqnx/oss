#!/bin/sh
# Pure-Python libraries, each with known answers: lark parses an arithmetic
# grammar (LALR and Earley) into the expected tree and evaluates it; ply
# builds a lexer and an LALR parser for a calculator; tomli parses a TOML
# document; pathspec matches gitignore patterns; trove-classifiers knows the
# real classifiers; typing_extensions, attrs, cattrs (structure/unstructure)
# and typeguard (type errors caught at run time); platformdirs and appdirs
# give the Unix directories; filelock excludes a second process and times
# out.
echo "== python3-pylibs =="
PY=${QNX_PORT_PYTHON:-/usr/local/bin/python3}; [ -x "$PY" ] || PY=python3
# No __pycache__ left in the installation's site-packages (docs/porting-hazards.md 183)
export PYTHONDONTWRITEBYTECODE=1
T=$(mktemp -d "${TMPDIR:-/tmp}/qnxport-pylibs.XXXXXX") || { echo "FAIL python3-pylibs (no temporary directory)"; exit 1; }
trap 'cd /; rm -rf "$T"' EXIT
cd "$T" || exit 1
cat > calc_ply.py <<'P'
tokens = ("NUM", "PLUS", "TIMES", "LP", "RP")
t_PLUS = r"\+"
t_TIMES = r"\*"
t_LP = r"\("
t_RP = r"\)"
t_ignore = " "
def t_NUM(t):
    r"\d+"
    t.value = int(t.value)
    return t
def t_error(t):
    raise SyntaxError(t.value[0])
precedence = (("left", "PLUS"), ("left", "TIMES"))
def p_e_plus(p):
    "e : e PLUS e"
    p[0] = p[1] + p[3]
def p_e_times(p):
    "e : e TIMES e"
    p[0] = p[1] * p[3]
def p_e_paren(p):
    "e : LP e RP"
    p[0] = p[2]
def p_e_num(p):
    "e : NUM"
    p[0] = p[1]
def p_error(p):
    raise SyntaxError(p)
P
cat > t.py <<'P'
import sys, os, subprocess
fails = 0
def check(ok, what):
    global fails
    print(("  ok   " if ok else "  FAIL ") + what); fails += (not ok)
def info(s): print("  info " + s)
def run(name, fn):
    try: fn()
    except Exception as e:
        import traceback; traceback.print_exc(limit=4, file=sys.stdout)
        check(False, "%s: %r" % (name, e))

def t_lark():
    import lark
    from lark import Lark, Transformer
    g = r"""
        ?start: sum
        ?sum: product | sum "+" product -> add | sum "-" product -> sub
        ?product: atom | product "*" atom -> mul | product "/" atom -> div
        ?atom: NUMBER -> number | "-" atom -> neg | "(" sum ")"
        %import common.NUMBER
        %import common.WS_INLINE
        %ignore WS_INLINE
    """
    class Calc(Transformer):
        number = lambda self, a: float(a[0])
        add = lambda self, a: a[0] + a[1]; sub = lambda self, a: a[0] - a[1]
        mul = lambda self, a: a[0] * a[1]; div = lambda self, a: a[0] / a[1]
        neg = lambda self, a: -a[0]
    for algo in ("lalr", "earley"):
        p = Lark(g, parser=algo)
        tree = p.parse("2 * (3 + 4) - -1")
        check(tree.data == "sub" and tree.children[0].data == "mul", "lark %s: tree shape sub(mul(..), neg(..))" % algo)
        check(Calc().transform(tree) == 15.0, "lark %s: 2*(3+4)--1 evaluates to 15" % algo)
    try:
        Lark(g, parser="lalr").parse("2 + * 3"); check(False, "lark: syntax error raises")
    except lark.exceptions.UnexpectedInput as e:
        check(e.column == 5, "lark: UnexpectedInput at column 5")
    info("lark %s" % lark.__version__)
run("lark", t_lark)

def t_ply():
    # ply reads the grammar from a module's source (docstrings, line numbers):
    # calc_ply.py is written by the shell part of this test.
    import ply.lex as lex, ply.yacc as yacc
    import calc_ply as mod
    lexer = lex.lex(module=mod)
    lexer.input("12 + 3*(4+1)")
    check([t.type for t in lexer] == ["NUM", "PLUS", "NUM", "TIMES", "LP", "NUM", "PLUS", "NUM", "RP"], "ply.lex: token stream")
    parser = yacc.yacc(module=mod, write_tables=False, debug=False)
    check(parser.parse("12 + 3*(4+1)", lexer=lexer) == 27 and parser.parse("2*3+4", lexer=lexer) == 10, "ply.yacc: precedence, 12+3*(4+1) = 27")
run("ply", t_ply)

def t_tomli():
    import tomli, datetime
    d = tomli.loads('title = "QNX"\n[owner]\nborn = 1979-05-27T07:32:00Z\n[[ports]]\nname = "zstd"\nver = [1, 5, 6]\n[[ports]]\nname = "cython"\nok = true\nf = 6.5e-1\n')
    check(d["title"] == "QNX" and d["ports"][1] == {"name": "cython", "ok": True, "f": 0.65} and d["ports"][0]["ver"] == [1, 5, 6], "tomli: tables, arrays of tables, values")
    check(d["owner"]["born"] == datetime.datetime(1979, 5, 27, 7, 32, tzinfo=datetime.timezone.utc), "tomli: offset date-time")
    try:
        tomli.loads("a = 1\na = 2"); check(False, "tomli: duplicate key raises")
    except tomli.TOMLDecodeError:
        check(True, "tomli: duplicate key raises TOMLDecodeError")
run("tomli", t_tomli)

def t_pathspec():
    import pathspec
    spec = pathspec.PathSpec.from_lines("gitwildmatch", ["*.o", "build/", "!keep.o", "/top.txt", "docs/**/*.md"])
    files = ["a.o", "src/b.o", "keep.o", "build/x.c", "top.txt", "sub/top.txt", "docs/a/b/c.md", "docs/readme.txt", "main.c"]
    check(sorted(spec.match_files(files)) == ["a.o", "build/x.c", "docs/a/b/c.md", "src/b.o", "top.txt"], "pathspec: gitignore patterns, negation, anchoring, **")
run("pathspec", t_pathspec)

def t_trove():
    import trove_classifiers as tc
    check("Operating System :: POSIX" in tc.classifiers and "License :: OSI Approved :: MIT License" in tc.classifiers, "trove-classifiers: real classifiers listed")
    check("Operating System :: QNX Nonsense" not in tc.classifiers and len(tc.classifiers) > 800, "trove-classifiers: invented one absent, > 800 known")
run("trove-classifiers", t_trove)

def t_typing():
    import typing_extensions as te
    class P(te.TypedDict):
        a: int
        b: te.NotRequired[str]
    check(P.__required_keys__ == frozenset({"a"}) and P.__optional_keys__ == frozenset({"b"}), "typing_extensions: TypedDict with NotRequired")
    @te.runtime_checkable
    class Closer(te.Protocol):
        def close(self) -> None: ...
    check(isinstance(open(os.devnull), Closer) and not isinstance(3, Closer), "typing_extensions: runtime-checkable Protocol")
    check(te.get_origin(te.Annotated[int, "x"]) is te.Annotated and te.get_args(te.Literal[1, 2]) == (1, 2), "typing_extensions: Annotated, Literal introspection")
run("typing-extensions", t_typing)

def t_attrs():
    import attrs, cattrs
    from typing import List, Optional
    @attrs.define
    class Port:
        name: str
        version: str = "0"
        deps: List[str] = attrs.field(factory=list)
        size: int = attrs.field(default=0, validator=attrs.validators.ge(0))
    @attrs.frozen
    class Repo:
        ports: List[Port]
        owner: Optional[str] = None
    p = Port("zstd", "1.5.6", ["cmake"], 3)
    check(repr(p) == "Port(name='zstd', version='1.5.6', deps=['cmake'], size=3)" and p == Port("zstd", "1.5.6", ["cmake"], 3), "attrs: __repr__ and __eq__ generated")
    check(attrs.asdict(p) == {"name": "zstd", "version": "1.5.6", "deps": ["cmake"], "size": 3} and attrs.evolve(p, size=9).size == 9, "attrs: asdict, evolve")
    try:
        Port("x", size=-1); check(False, "attrs: validator rejects -1")
    except ValueError:
        check(True, "attrs: validator rejects -1")
    r = Repo([p, Port("lark")])
    try:
        r.owner = "x"; check(False, "attrs: frozen class")
    except attrs.exceptions.FrozenInstanceError:
        check(True, "attrs: frozen instance refuses assignment")
    raw = cattrs.unstructure(r)
    check(raw == {"ports": [{"name": "zstd", "version": "1.5.6", "deps": ["cmake"], "size": 3}, {"name": "lark", "version": "0", "deps": [], "size": 0}], "owner": None}, "cattrs: unstructure nested attrs classes")
    check(cattrs.structure(raw, Repo) == r, "cattrs: structure back to equal objects")
    check(cattrs.structure({"name": "n", "size": "12"}, Port).size == 12, "cattrs: '12' structured to int 12")
    try:
        cattrs.structure({"ports": [{"size": 1}]}, Repo); check(False, "cattrs: missing field raises")
    except Exception as e:
        check(type(e).__name__ in ("ClassValidationError", "IterableValidationError", "KeyError"), "cattrs: missing field raises %s" % type(e).__name__)
run("attrs/cattrs", t_attrs)

def t_typeguard():
    import typeguard
    from typing import List, Dict, Union
    check(typeguard.check_type([1, 2], List[int]) == [1, 2], "typeguard: check_type passes a List[int]")
    try:
        typeguard.check_type({"a": "x"}, Dict[str, int]); check(False, "typeguard: Dict[str, int] with a str value")
    except typeguard.TypeCheckError as e:
        check("int" in str(e), "typeguard: Dict[str, int] with a str value raises TypeCheckError")
    @typeguard.typechecked
    def f(x: Union[int, str]) -> int:
        return len(x) if isinstance(x, str) else x
    check(f("abc") == 3 and f(4) == 4, "typeguard: @typechecked accepts valid calls")
    try:
        f(1.5); check(False, "typeguard: @typechecked rejects float")
    except typeguard.TypeCheckError:
        check(True, "typeguard: @typechecked rejects a float argument")
run("typeguard", t_typeguard)

def t_dirs():
    import platformdirs, appdirs
    home = os.path.expanduser("~")
    os.environ.pop("XDG_CONFIG_HOME", None); os.environ.pop("XDG_CACHE_HOME", None); os.environ.pop("XDG_DATA_HOME", None)
    d = platformdirs.PlatformDirs("qnxapp", "qnx", version="1.0")
    info("platformdirs %s: %s, %s" % (platformdirs.__version__, d.user_config_dir, d.user_cache_dir))
    check(d.user_config_dir == home + "/.config/qnxapp/1.0" and d.user_cache_dir == home + "/.cache/qnxapp/1.0", "platformdirs: XDG config and cache directories")
    check(platformdirs.user_data_dir("qnxapp") == home + "/.local/share/qnxapp", "platformdirs: user data directory")
    os.environ["XDG_CONFIG_HOME"] = "/tmp/xdgq"
    check(platformdirs.user_config_dir("a") == "/tmp/xdgq/a", "platformdirs: XDG_CONFIG_HOME honoured")
    check(appdirs.user_data_dir("qnxapp", "qnx") == home + "/.local/share/qnxapp" and appdirs.user_config_dir("a") == "/tmp/xdgq/a", "appdirs: data directory, XDG_CONFIG_HOME")
run("platformdirs/appdirs", t_dirs)

def t_filelock():
    import filelock
    lp = os.path.join(os.getcwd(), "x.lock")
    lock = filelock.FileLock(lp, timeout=0.5)
    with lock:
        check(lock.is_locked, "filelock: acquired")
        r = subprocess.run([sys.executable, "-c", "import filelock, sys\ntry:\n  filelock.FileLock(sys.argv[1], timeout=0.5).acquire(); print('got')\nexcept filelock.Timeout: print('timeout')", lp], capture_output=True, text=True)
        check(r.stdout.strip() == "timeout", "filelock: another process times out while it is held")
        with lock:
            check(lock.lock_counter == 2, "filelock: reentrant in the same thread")
    check(not lock.is_locked, "filelock: released")
    r = subprocess.run([sys.executable, "-c", "import filelock, sys\nwith filelock.FileLock(sys.argv[1], timeout=2): print('got')", lp], capture_output=True, text=True)
    check(r.stdout.strip() == "got", "filelock: another process acquires it after release")
    info("filelock %s, %s" % (filelock.__version__, type(lock).__name__))
run("filelock", t_filelock)
sys.exit(1 if fails else 0)
P
"$PY" t.py; rc=$?
[ $rc -eq 0 ] && { echo "PASS python3-pylibs"; exit 0; }
echo "FAIL python3-pylibs (rc=$rc)"; exit 1
