#!/bin/sh
# python3-six: the Python 2/3 compatibility layer as old code uses it --
# type aliases, six.moves imports, byte/text helpers, metaclass helpers,
# dictionary iterators, and re-raising with a cause.
echo "== python3-six =="
PY=${QNX_PORT_PYTHON:-/usr/local/bin/python3}; [ -x "$PY" ] || PY=python3
T=$(mktemp -d "${TMPDIR:-/tmp}/qnxport-six.XXXXXX") || { echo "FAIL python3-six (no temporary directory)"; exit 1; }
cd "$T" || exit 1
cat > t.py <<'P'
import sys, os
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
import six
info("six %s" % six.__version__)
def basics():
    check(six.PY3 and not six.PY2 and six.string_types == (str,) and six.integer_types == (int,), "PY3, string_types, integer_types")
    check(six.text_type is str and six.binary_type is bytes, "text_type, binary_type")
    check(six.ensure_binary(u"grüß") == b"gr\xc3\xbc\xc3\x9f" and six.ensure_text(b"qnx") == u"qnx" and six.ensure_str(b"x") == "x", "ensure_binary/text/str (UTF-8)")
    check(six.b("abc") == b"abc" and six.u("abc") == u"abc" and six.int2byte(65) == b"A" and six.byte2int(b"A") == 65, "b, u, int2byte, byte2int")
def moves():
    from six.moves.urllib.parse import quote, urlparse
    from six.moves import range, configparser, queue, reduce
    check(quote("a b/c") == "a%20b/c" and urlparse("http://qnx.com:80/x").port == 80, "six.moves.urllib.parse")
    check(list(range(3)) == [0, 1, 2] and reduce(lambda a, b: a * b, range(1, 6)) == 120, "six.moves range, reduce")
    q = queue.Queue(); q.put(1); check(q.get() == 1, "six.moves.queue")
    c = configparser.ConfigParser(); c.read_string(u"[s]\nk = v\n"); check(c.get("s", "k") == "v", "six.moves.configparser")
    from six.moves import cPickle
    check(cPickle.loads(cPickle.dumps({"a": 1})) == {"a": 1}, "six.moves.cPickle")
def meta():
    class Meta(type):
        def __new__(m, name, bases, ns): ns["tagged"] = name.lower(); return type.__new__(m, name, bases, ns)
    @six.add_metaclass(Meta)
    class A(object): pass
    class B(six.with_metaclass(Meta, object)): pass
    check(A.tagged == "a" and B.tagged == "b" and type(A) is Meta and type(B) is Meta, "add_metaclass, with_metaclass")
    d = {"x": 1, "y": 2}
    check(sorted(six.iteritems(d)) == [("x", 1), ("y", 2)] and sorted(six.iterkeys(d)) == ["x", "y"], "iteritems, iterkeys")
    @six.python_2_unicode_compatible
    class S(object):
        def __str__(self): return u"qnx"
    check(str(S()) == "qnx", "python_2_unicode_compatible")
def raising():
    try:
        try: raise KeyError("inner")
        except KeyError as e: six.raise_from(ValueError("outer"), e)
    except ValueError as e:
        check(isinstance(e.__cause__, KeyError), "raise_from keeps the cause")
    try: six.reraise(TypeError, TypeError("again"))
    except TypeError as e: check(str(e) == "again", "reraise")
for n, f in (("basics", basics), ("moves", moves), ("metaclass", meta), ("raise", raising)):
    run(n, f)
sys.exit(1 if fails else 0)
P
"$PY" t.py; rc=$?
cd /; rm -rf "$T"
[ $rc -eq 0 ] && { echo "PASS python3-six"; exit 0; }
echo "FAIL python3-six (rc=$rc)"; exit 1
