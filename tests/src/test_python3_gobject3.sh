#!/bin/sh
# python3-gobject3 (PyGObject) with glib-typelibs: GLib, GObject and Gio
# load from their typelibs; a GLib main loop runs an idle callback, then a
# timeout, then a watch on a pipe (GLib.io_add_watch: only correct if the
# typelib's GIOCondition values are QNX's poll() bits, see glib-typelibs);
# a GObject subclass emits its own signal with an argument and notifies a
# property change; GVariant, markup escaping, Gio files and the cairo
# foreign struct support work; the IOCondition and SocketFamily values equal
# Python's select/socket constants.
echo "== python3-gobject3 =="
PY=${QNX_PORT_PYTHON:-/usr/local/bin/python3}; [ -x "$PY" ] || PY=python3
# No __pycache__ left in the installation's site-packages (docs/porting-hazards.md 183)
export PYTHONDONTWRITEBYTECODE=1
T=$(mktemp -d "${TMPDIR:-/tmp}/qnxport-pygobject.XXXXXX") || { echo "FAIL python3-gobject3 (no temporary directory)"; exit 1; }
trap 'cd /; rm -rf "$T"' EXIT
cd "$T" || exit 1
cat > t.py <<'P'
import os, select, socket, sys
fails = 0
def check(ok, what):
    global fails
    print(("  ok   " if ok else "  FAIL ") + what); fails += (not ok)
def info(s): print("  info " + s)
import gi
gi.require_version("GLib", "2.0"); gi.require_version("GObject", "2.0"); gi.require_version("Gio", "2.0")
from gi.repository import GLib, GObject, Gio
info("PyGObject %s, GLib %d.%d.%d" % (gi.__version__, GLib.MAJOR_VERSION, GLib.MINOR_VERSION, GLib.MICRO_VERSION))
check((GLib.MAJOR_VERSION, GLib.MINOR_VERSION, GLib.MICRO_VERSION) == (2, 82, 4), "GLib typelib is the ported GLib 2.82.4")
check(int(GLib.IOCondition.IN) == select.POLLIN and int(GLib.IOCondition.OUT) == select.POLLOUT and int(GLib.IOCondition.HUP) == select.POLLHUP,
      "GLib.IOCondition values are the target's poll() bits")
check(int(Gio.SocketFamily.IPV6) == socket.AF_INET6, "Gio.SocketFamily.IPV6 is the target's AF_INET6")
loop = GLib.MainLoop()
ev = []
# Each step starts the next, so the order does not depend on how fast the
# machine is: a 50 ms timeout added before loop.run() can already be due at
# the first iteration on a slow emulated target, and a due timeout
# (PRIORITY_DEFAULT) goes before an idle callback (PRIORITY_DEFAULT_IDLE).
def on_idle():
    ev.append("idle"); GLib.timeout_add(50, on_timeout); return False
GLib.idle_add(on_idle)
def on_timeout():
    ev.append("timeout"); os.write(w, b"x"); return False
r, w = os.pipe()
def on_io(fd, cond):
    ev.append(("io", os.read(fd, 1), bool(cond & GLib.IOCondition.IN))); loop.quit(); return False
GLib.io_add_watch(r, GLib.PRIORITY_DEFAULT, GLib.IOCondition.IN, on_io)
GLib.timeout_add_seconds(10, loop.quit)
loop.run()
check(ev == ["idle", "timeout", ("io", b"x", True)], "main loop: idle, timeout, then the pipe watch")
if ev != ["idle", "timeout", ("io", b"x", True)]: info(repr(ev))
class Counter(GObject.Object):
    __gsignals__ = {"bumped": (GObject.SignalFlags.RUN_FIRST, None, (int,))}
    value = GObject.Property(type=int, default=0)
    def bump(self, n):
        self.value += n; self.emit("bumped", self.value)
c = Counter(); got = []; notes = []
c.connect("bumped", lambda o, v: got.append(v))
c.connect("notify::value", lambda o, p: notes.append(p.name))
c.bump(40); c.bump(2)
check(got == [40, 42] and c.props.value == 42, "GObject subclass: own signal with an int argument")
check(notes == ["value", "value"], "GObject property notify")
check(GObject.type_name(Counter.__gtype__).endswith("Counter") and GObject.type_is_a(Counter.__gtype__, GObject.Object.__gtype__), "the subclass is a registered GType")
v = GLib.Variant("(sia{sv})", ("qnx", 8, {"k": GLib.Variant("b", True)}))
check(v.print_(True) == "('qnx', 8, {'k': <true>})" and v.unpack() == ("qnx", 8, {"k": True}), "GVariant print and unpack")
check(GLib.markup_escape_text("<a&b>") == "&lt;a&amp;b&gt;", "GLib.markup_escape_text")
with open("f.txt", "w") as f: f.write("hello gio\n")
ok_, data, etag = Gio.File.new_for_path(os.path.abspath("f.txt")).load_contents()
check(ok_ and data == b"hello gio\n", "Gio.File.load_contents")
check(GLib.compute_checksum_for_string(GLib.ChecksumType.SHA256, "abc", -1) == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad", "GLib SHA-256 of \"abc\"")
try:
    gi.require_foreign("cairo"); check(True, "cairo foreign struct support (gi._gi_cairo with pycairo)")
except ImportError as e:
    check(False, "cairo foreign struct support: %s" % e)
sys.exit(1 if fails else 0)
P
"$PY" t.py; rc=$?
[ $rc -eq 0 ] && { echo "PASS python3-gobject3"; exit 0; }
echo "FAIL python3-gobject3 (rc=$rc)"; exit 1
