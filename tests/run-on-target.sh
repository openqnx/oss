#!/bin/sh
# Run the port test suite ON a QNX target.
#
# Copy stage/<arch>/usr/local to the target (it is a /usr/local overlay), then:
#     export LD_LIBRARY_PATH=/usr/local/lib
#     /usr/local/bin/qnx-port-tests/run-on-target.sh [test_name...]
#
# Each test prints per-check lines and a final PASS/FAIL; this script tallies
# them and exits non-zero if any test failed.
DIR=$(cd "$(dirname "$0")" && pwd)
LD_LIBRARY_PATH="$DIR/../lib:/usr/local/lib:$LD_LIBRARY_PATH"; export LD_LIBRARY_PATH
# Components with a prefix of their own (/usr/local/opt/<name>: TensorFlow
# Lite, PyTorch, ROS 2), in the bundle and installed.
for d in "$DIR"/../opt/*/lib "$DIR"/../opt/ros/*/lib /usr/local/opt/*/lib /usr/local/opt/ros/*/lib; do
  [ -d "$d" ] && LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$d"
done
# ROS finds its type support libraries and nodes through the ament index.
for d in "$DIR"/../opt/ros/* /usr/local/opt/ros/*; do
  [ -d "$d/share/ament_index" ] && { AMENT_PREFIX_PATH="$(cd "$d" && pwd)"; export AMENT_PREFIX_PATH; break; }
done
# Data some libraries read at runtime, carried in the bundle next to bin/.
[ -d "$DIR/../share/X11/xkb" ] && { XKB_CONFIG_ROOT="$DIR/../share/X11/xkb"; export XKB_CONFIG_ROOT; }
[ -d "$DIR/../etc/fonts" ] && { FONTCONFIG_PATH="$DIR/../etc/fonts"; export FONTCONFIG_PATH; }
[ -d "$DIR/../share/terminfo" ] && { TERMINFO="$DIR/../share/terminfo"; export TERMINFO; }
# libthai compiles in an absolute path to its word-break dictionary, which is
# not where a relocatable bundle unpacks; LIBTHAI_DICTDIR overrides it.
[ -d "$DIR/../share/libthai" ] && { LIBTHAI_DICTDIR="$DIR/../share/libthai"; export LIBTHAI_DICTDIR; }
# GStreamer elements are loadable modules found through GST_PLUGIN_PATH, and
# the registry cache must be writable or every run rescans and warns.
if [ -d "$DIR/../lib/gstreamer-1.0" ]; then
  GST_PLUGIN_PATH="$DIR/../lib/gstreamer-1.0"; export GST_PLUGIN_PATH
  GST_PLUGIN_SYSTEM_PATH="$DIR/../lib/gstreamer-1.0"; export GST_PLUGIN_SYSTEM_PATH
  # The registry is a cache that also remembers plugins which FAILED to load,
  # and keeps them out while their files look unchanged.  A fixed path in /tmp
  # outlived the run: plugins that failed once (a missing codec library, since
  # fixed) stayed unregistered in every later batch.  Keep it in the unpacked
  # tree, which is new each time.
  GST_REGISTRY="$DIR/../gst-registry.bin"; export GST_REGISTRY
  [ -x "$DIR/../libexec/gstreamer-1.0/gst-plugin-scanner" ] && {
    GST_PLUGIN_SCANNER="$DIR/../libexec/gstreamer-1.0/gst-plugin-scanner"
    export GST_PLUGIN_SCANNER
  }
fi
# The bundled CPython stdlib.  test_python runs with an isolated config, which
# ignores PYTHONHOME, so it reads this variable itself and sets PyConfig.home.
# Installed from packages (apk add qnx-test-python) the stdlib is the python3
# package's, under /usr/local.
if [ -d "$DIR/../lib/python3.14" ]; then QNX_PORT_PYTHONHOME="$(cd "$DIR/.." && pwd)"; export QNX_PORT_PYTHONHOME
elif [ -d /usr/local/lib/python3.14 ]; then QNX_PORT_PYTHONHOME=/usr/local; export QNX_PORT_PYTHONHOME; fi
rm -rf /tmp/qnx-port-tests-run.* 2>/dev/null   # stale runtime dirs from earlier runs
# Wayland refuses an XDG_RUNTIME_DIR that is not a 0700 directory owned by the
# caller, so /tmp itself will not do -- make a private one.
if [ -z "$XDG_RUNTIME_DIR" ] || [ ! -d "$XDG_RUNTIME_DIR" ]; then
  XDG_RUNTIME_DIR=/tmp/qnx-port-tests-run.$$
  mkdir -p "$XDG_RUNTIME_DIR" && chmod 700 "$XDG_RUNTIME_DIR"
  export XDG_RUNTIME_DIR
fi
# Tests write scratch files under /tmp, and a small QNX image fills up fast: a
# 61 MB /data reached 100% after a few runs, and then every test that writes a
# file failed -- a growing, misleading failure list that had nothing to do with
# the ports.  Clear our own leftovers before and after, and say so if the
# filesystem is nearly full anyway.
# NB: not /tmp/qnx-port-tests-run.* -- that is the XDG_RUNTIME_DIR this script
# created a few lines above, and wiping it made every Wayland socket fail with
# "unable to open lockfile".  Stale ones from earlier runs are cleared before
# that directory is set up.
qnxport_clean_tmp() { rm -rf /tmp/qnxport-* /tmp/test_*.out 2>/dev/null; }
qnxport_clean_tmp
free_kb=$(df -kP /tmp 2>/dev/null | awk 'NR==2 {print $4}')
[ -n "$free_kb" ] && [ "$free_kb" -lt 20480 ] && \
  echo "!! only $((free_kb/1024)) MB free on /tmp -- tests that write files may fail"

pass=0; fail=0; failed=""
list="$DIR/tests.list"
[ -f "$list" ] || list=$(ls "$DIR"/test_* 2>/dev/null | sed 's|.*/||')
# Named tests only: run-on-target.sh test_zstd test_jq (scripts/verify-on-target).
[ $# -gt 0 ] && list="$*"
for t in $(cat "$list" 2>/dev/null || echo $list); do
  [ -x "$DIR/$t" ] || continue
  "$DIR/$t" > /tmp/$t.out 2>&1
  rc=$?
  if [ $rc -eq 0 ]; then pass=$((pass+1)); echo "PASS $t"
  else fail=$((fail+1)); failed="$failed $t"; echo "FAIL $t (rc=$rc)"; sed 's/^/     /' /tmp/$t.out; fi
done
qnxport_clean_tmp
echo "--------------------------------------------"
echo "passed: $pass   failed: $fail"
[ $fail -eq 0 ] || { echo "failing:$failed"; exit 1; }
exit 0
