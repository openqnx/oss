#!/bin/sh
# python3-dbus (dbus-python) and python3-pydbus on a private dbus-daemon:
#  - a dbus-python service (dbus.service.Object on the GLib main loop of
#    dbus.mainloop.glib) is called from another process with dbus-python:
#    Add(2, 40) = 42, Upper("qnx") = "QNX", its introspection XML, and a
#    signal it emits arrives at the client's main loop;
#  - pydbus (GDBus through PyGObject) calls the same service, and publishes
#    its own object, which dbus-python calls back;
#  - a call to a missing method raises the D-Bus UnknownMethod error.
echo "== python3-dbus =="
PY=${QNX_PORT_PYTHON:-/usr/local/bin/python3}; [ -x "$PY" ] || PY=python3
BIN=$(dirname "$PY")
# No __pycache__ left in the installation's site-packages (docs/porting-hazards.md 183)
export PYTHONDONTWRITEBYTECODE=1
T=$(mktemp -d "${TMPDIR:-/tmp}/qnxport-pydbus.XXXXXX") || { echo "FAIL python3-dbus (no temporary directory)"; exit 1; }
PIDS=""
trap 'for p in $PIDS; do kill $p 2>/dev/null; done; cd /; rm -rf "$T"' EXIT
cd "$T" || exit 1
fails=0
ok()  { echo "  ok   $1"; }
bad() { echo "  FAIL $1"; fails=$((fails+1)); }
DAEMON="$BIN/dbus-daemon"; [ -x "$DAEMON" ] || DAEMON=dbus-daemon
cat > bus.conf <<EOS
<!DOCTYPE busconfig PUBLIC "-//freedesktop//DTD D-Bus Bus Configuration 1.0//EN"
 "http://www.freedesktop.org/standards/dbus/1.0/busconfig.dtd">
<busconfig><type>session</type><listen>unix:path=$T/bus</listen><allow_anonymous/>
<policy context="default"><allow send_destination="*" eavesdrop="true"/><allow eavesdrop="true"/><allow own="*"/></policy>
</busconfig>
EOS
"$DAEMON" --config-file="$T/bus.conf" --nofork --nopidfile > bus.log 2>&1 &
PIDS="$PIDS $!"
i=0; while [ ! -S "$T/bus" ] && [ $i -lt 50 ]; do sleep 1; i=$((i+1)); done
[ -S "$T/bus" ] || { bad "private dbus-daemon: $(cat bus.log)"; echo "FAIL python3-dbus"; exit 1; }
export DBUS_SESSION_BUS_ADDRESS=unix:path=$T/bus
cat > server.py <<'P'
import dbus, dbus.service, dbus.mainloop.glib
from gi.repository import GLib
dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
loop = GLib.MainLoop()
class Calc(dbus.service.Object):
    @dbus.service.method("org.qnx.Calc", in_signature="ii", out_signature="i")
    def Add(self, a, b): return a + b
    @dbus.service.method("org.qnx.Calc", in_signature="s", out_signature="s")
    def Upper(self, s): return s.upper()
    @dbus.service.method("org.qnx.Calc", in_signature="s", out_signature="")
    def Ping(self, s): self.Pinged(s + "!")
    @dbus.service.signal("org.qnx.Calc", signature="s")
    def Pinged(self, s): pass
    @dbus.service.method("org.qnx.Calc", in_signature="", out_signature="")
    def Quit(self): GLib.idle_add(loop.quit)
bus = dbus.SessionBus()
name = dbus.service.BusName("org.qnx.Calc", bus)
Calc(bus, "/org/qnx/Calc")
GLib.timeout_add_seconds(120, loop.quit)
print("ready", flush=True)
loop.run()
P
cat > pyserver.py <<'P'
from pydbus import SessionBus
from gi.repository import GLib
loop = GLib.MainLoop()
class Greeter(object):
    """
    <node><interface name="org.qnx.Greeter">
      <method name="Hello"><arg type="s" name="who" direction="in"/><arg type="s" name="r" direction="out"/></method>
      <method name="Quit"/>
    </interface></node>
    """
    def Hello(self, who): return "hello " + who
    def Quit(self): GLib.idle_add(loop.quit)
SessionBus().publish("org.qnx.Greeter", Greeter())
GLib.timeout_add_seconds(120, loop.quit)
print("ready", flush=True)
loop.run()
P
cat > client.py <<'P'
import sys, time
fails = 0
def check(ok, what):
    global fails
    print(("  ok   " if ok else "  FAIL ") + what); fails += (not ok)
import dbus, dbus.mainloop.glib
from gi.repository import GLib
dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
bus = dbus.SessionBus()
for i in range(100):
    if bus.name_has_owner("org.qnx.Calc") and bus.name_has_owner("org.qnx.Greeter"): break
    time.sleep(0.1)
calc = dbus.Interface(bus.get_object("org.qnx.Calc", "/org/qnx/Calc"), "org.qnx.Calc")
check(calc.Add(2, 40) == 42 and isinstance(calc.Add(2, 40), dbus.Int32), "dbus-python: Add(2, 40) = 42 (an Int32)")
check(calc.Upper("qnx") == "QNX", "dbus-python: Upper(\"qnx\")")
xml = bus.get_object("org.qnx.Calc", "/org/qnx/Calc").Introspect(dbus_interface="org.freedesktop.DBus.Introspectable")
check('<method name="Add">' in xml and '<signal name="Pinged">' in xml, "dbus-python: introspection XML of the service")
got = []
loop = GLib.MainLoop()
bus.add_signal_receiver(lambda s: (got.append(s), loop.quit()), signal_name="Pinged", dbus_interface="org.qnx.Calc")
calc.Ping("pong")
GLib.timeout_add_seconds(10, loop.quit)
loop.run()
check(got == ["pong!"], "dbus-python: the service's signal arrives in the main loop")
try:
    bus.get_object("org.qnx.Calc", "/org/qnx/Calc").Nope(dbus_interface="org.qnx.Calc")
    check(False, "missing method raises UnknownMethod")
except dbus.DBusException as e:
    check(e.get_dbus_name() == "org.freedesktop.DBus.Error.UnknownMethod", "missing method raises UnknownMethod")
import pydbus
pb = pydbus.SessionBus()
pc = pb.get("org.qnx.Calc", "/org/qnx/Calc")
check(pc.Add(1, 2) == 3 and pc.Upper("abc") == "ABC", "pydbus: calls the dbus-python service")
check("org.qnx.Calc" in pb.get(".DBus").ListNames(), "pydbus: org.freedesktop.DBus.ListNames")
greeter = dbus.Interface(bus.get_object("org.qnx.Greeter", "/org/qnx/Greeter"), "org.qnx.Greeter")
check(greeter.Hello("qnx") == "hello qnx", "dbus-python calls the object pydbus published")
calc.Quit(); greeter.Quit()
sys.exit(1 if fails else 0)
P
"$PY" server.py > server.out 2>&1 &
PIDS="$PIDS $!"
"$PY" pyserver.py > pyserver.out 2>&1 &
PIDS="$PIDS $!"
"$PY" client.py; rc=$?
[ $rc -eq 0 ] || { fails=$((fails+1)); sed 's/^/  info server: /' server.out; sed 's/^/  info pyserver: /' pyserver.out; }
[ $fails -eq 0 ] && { echo "PASS python3-dbus"; exit 0; }
echo "FAIL python3-dbus ($fails)"; exit 1
