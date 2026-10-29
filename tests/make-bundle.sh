#!/bin/bash
# Build a minimal tarball for running the port tests on a QNX target: the test
# binaries plus only the shared libraries they actually pull in (resolved
# transitively through DT_NEEDED), rather than the whole 1.5 GB staging tree.
set -o pipefail
WS="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

# With no argument, bundle every architecture -- defaulting to just one is how
# a stale bundle gets shipped to a target and blamed on the port.
if [ $# -eq 0 ]; then
  rc=0
  for a in aarch64le x86_64; do "$0" "$a" || rc=1; done
  exit $rc
fi
ARCH="$1"
source "$WS/env.sh" "$ARCH" >/dev/null || exit 1
READELF="$QNX_HOST/usr/bin/nto${QNX_CPU}-readelf"
# BUNDLE_STAGE / BUNDLE_OUT: bundle a different staging tree (e.g. the previous
# one while a clean rebuild is rewriting stage/) without disturbing either.
SRC="${BUNDLE_STAGE:-$STAGE}$PREFIX"
OUT="${BUNDLE_OUT:-$WS/dist}/qnx-port-tests-$ARCH"
rm -rf "$OUT"; mkdir -p "$OUT/bin" "$OUT/lib"

cp -a "$SRC/bin/qnx-port-tests/." "$OUT/bin/"
# Take the runner from the source tree, not from staging: staging only gets a
# copy when someone remembers to put one there, and a stale runner silently
# drops LD_LIBRARY_PATH and fails every test.
cp -a "$WS/tests/run-on-target.sh" "$OUT/bin/"

# Walk DT_NEEDED from every test binary; keep only libraries we staged
# ourselves -- anything from the SDP is already present on the target.
need() { "$READELF" -d "$1" 2>/dev/null | sed -n 's/.*NEEDED.*\[\(.*\)\]/\1/p'; }

# Copy a library and every step of its symlink chain. Chains are more than one
# link deep (libncursesw.so -> libncursesw.so.6 -> libncursesw.so.6.5), so
# copying just the head and the resolved file leaves the middle link dangling
# and the loader fails at run time with "Could not load library".
copy_chain() {
  local p="$1"
  while [ -L "$p" ]; do
    cp -a "$p" "$OUT/lib/" 2>/dev/null
    local t; t=$(readlink "$p")
    case "$t" in /*) p="$t" ;; *) p="$(dirname "$p")/$t" ;; esac
  done
  cp -a "$p" "$OUT/lib/" 2>/dev/null
}
declare -A seen
queue=$(ls "$OUT"/bin/test_* 2>/dev/null)
while [ -n "$queue" ]; do
  next=""
  for f in $queue; do
    for n in $(need "$f"); do
      [ -n "${seen[$n]}" ] && continue
      if [ -e "$SRC/lib/$n" ]; then
        seen[$n]=1
        # A DT_NEEDED name is usually a symlink (libfmt.so.11 -> libfmt.so.11.2.0).
        # Copying only the link leaves it dangling on the target and the loader
        # reports "Could not load library", so bring the real file across too and
        # keep walking the *resolved* file for its own dependencies.
        real=$(readlink -f "$SRC/lib/$n")
        copy_chain "$SRC/lib/$n"
        next="$next $real"
      fi
    done
  done
  queue="$next"
done

# Also carry the unversioned .so development symlinks whose target we packaged.
for l in "$SRC"/lib/*.so; do
  [ -L "$l" ] || continue
  t=$(basename "$(readlink -f "$l")")
  [ -e "$OUT/lib/$t" ] && copy_chain "$l"
done

# GStreamer elements live in loadable modules, not in DT_NEEDED entries, so the
# dependency walk above never sees them. Without these the registry comes up
# with one plugin ("staticelements") and no element can be created.
if [ -d "$SRC/lib/gstreamer-1.0" ]; then
  mkdir -p "$OUT/lib/gstreamer-1.0"
  cp -a "$SRC"/lib/gstreamer-1.0/*.so "$OUT/lib/gstreamer-1.0/" 2>/dev/null
  # Each plugin drags in its own codec libraries; walk those too.
  queue=$(ls "$OUT"/lib/gstreamer-1.0/*.so 2>/dev/null)
  while [ -n "$queue" ]; do
    next=""
    for f in $queue; do
      for n in $(need "$f"); do
        [ -n "${seen[$n]}" ] && continue
        if [ -e "$SRC/lib/$n" ]; then
          seen[$n]=1
          copy_chain "$SRC/lib/$n"
          next="$next $(readlink -f "$SRC/lib/$n")"
        fi
      done
    done
    queue="$next"
  done
fi

# Enchant's spell-checking backends are the same shape as GStreamer's: dlopened
# modules under lib/enchant-2, invisible to the DT_NEEDED walk.  Without them
# the broker constructs happily and reports no providers at all.
if [ -d "$SRC/lib/enchant-2" ]; then
  mkdir -p "$OUT/lib/enchant-2"
  cp -a "$SRC"/lib/enchant-2/*.so "$OUT/lib/enchant-2/" 2>/dev/null
  queue=$(ls "$OUT"/lib/enchant-2/*.so 2>/dev/null)
  while [ -n "$queue" ]; do
    next=""
    for f in $queue; do
      for n in $(need "$f"); do
        [ -n "${seen[$n]}" ] && continue
        if [ -e "$SRC/lib/$n" ]; then
          seen[$n]=1
          copy_chain "$SRC/lib/$n"
          next="$next $(readlink -f "$SRC/lib/$n")"
        fi
      done
    done
    queue="$next"
  done
fi

# Libraries a port loads by name with dlopen() rather than linking: invisible
# to the DT_NEEDED walk too.  sdl2-compat is SDL2's API implemented on top of
# SDL3, which it dlopens at SDL_Init ("Failed loading SDL3 library").
for n in libSDL3.so.0; do
  [ -n "${seen[$n]}" ] || [ ! -e "$SRC/lib/$n" ] && continue
  seen[$n]=1; copy_chain "$SRC/lib/$n"
  queue=$(readlink -f "$SRC/lib/$n")
  while [ -n "$queue" ]; do
    next=""
    for f in $queue; do
      for d in $(need "$f"); do
        [ -n "${seen[$d]}" ] && continue
        if [ -e "$SRC/lib/$d" ]; then
          seen[$d]=1; copy_chain "$SRC/lib/$d"; next="$next $(readlink -f "$SRC/lib/$d")"
        fi
      done
    done
    queue="$next"
  done
done

# CPython's standard library, and the extension modules in lib-dynload with
# the libraries they link.  Without it an embedded interpreter goes looking for
# a stdlib elsewhere -- and on an image that ships QNX's own Python it FINDS one:
# with no program name, CPython searches PATH for python3, hits
# /system/bin/python3, and takes /system as its prefix.  The Pi run then passed
# test_python using QNX's prebuilt _ssl, _ctypes and _sqlite3, not ours.
# test_python now pins its home to the bundle and checks where modules came from.
PYV=python3.14
if [ -d "$SRC/lib/$PYV" ]; then
  mkdir -p "$OUT/lib"
  tar cf - -C "$SRC/lib" --exclude="$PYV/test" --exclude="$PYV/idlelib" \
      --exclude="$PYV/tkinter" --exclude="$PYV/turtledemo" --exclude="$PYV/ensurepip" \
      --exclude="$PYV/site-packages" --exclude='__pycache__' "$PYV" | tar xf - -C "$OUT/lib"
  queue=$(ls "$OUT/lib/$PYV/lib-dynload"/*.so 2>/dev/null)
  while [ -n "$queue" ]; do
    next=""
    for f in $queue; do
      for n in $(need "$f"); do
        [ -n "${seen[$n]}" ] && continue
        if [ -e "$SRC/lib/$n" ]; then
          seen[$n]=1
          copy_chain "$SRC/lib/$n"
          next="$next $(readlink -f "$SRC/lib/$n")"
        fi
      done
    done
    queue="$next"
  done
fi

# Components installed in a prefix of their own (/usr/local/opt/<name>), each
# carried whole when its test is in the bundle, with what it links from lib/.
# ROS brings share/ too: the ament index there is how it finds its type support
# libraries and nodes.
for e in test_tflite:opt/tensorflow-lite/lib test_torch:opt/pytorch/lib \
         test_ros2:opt/ros/jazzy/lib test_ros2:opt/ros/jazzy/share; do
  t=${e%%:*} d=${e#*:}
  [ -e "$OUT/bin/$t" ] && [ -d "$SRC/$d" ] || continue
  mkdir -p "$OUT/$d"; cp -a "$SRC/$d/." "$OUT/$d/"
  queue=$(find "$OUT/$d" -type f \( -name '*.so*' -o -perm -u+x \))
  while [ -n "$queue" ]; do
    next=""
    for f in $queue; do
      for n in $(need "$f"); do
        [ -n "${seen[$n]}" ] && continue
        if [ -e "$SRC/lib/$n" ]; then
          seen[$n]=1
          copy_chain "$SRC/lib/$n"
          next="$next $(readlink -f "$SRC/lib/$n")"
        fi
      done
    done
    queue="$next"
  done
done

# gst-plugin-scanner is a helper binary GStreamer forks to probe plugins out of
# process; without it the registry falls back to in-process loading and warns.
if [ -x "$SRC/libexec/gstreamer-1.0/gst-plugin-scanner" ]; then
  mkdir -p "$OUT/libexec/gstreamer-1.0"
  cp -a "$SRC/libexec/gstreamer-1.0/gst-plugin-scanner" "$OUT/libexec/gstreamer-1.0/"
fi

# Nothing may dangle, or the loader fails at run time.
dangling=0
for f in "$OUT"/lib/*; do [ -e "$f" ] || { echo "!! dangling: $(basename "$f")"; dangling=1; }; done
[ $dangling -eq 0 ] || { echo "!! bundle has dangling symlinks"; exit 1; }

# Runtime data some libraries read from disk. libxkbcommon will not even create
# a context without the XKB rule files, and fontconfig needs its config.
for d in share/X11/xkb etc/fonts share/fontconfig share/terminfo share/libthai; do
  if [ -d "$SRC/$d" ]; then
    mkdir -p "$OUT/$d"; cp -a "$SRC/$d/." "$OUT/$d/"
  fi
done

D="$(dirname "$OUT")"
tar czf "$D/qnx-port-tests-$ARCH.tar.gz" -C "$D" "qnx-port-tests-$ARCH"
echo "bundle: $D/qnx-port-tests-$ARCH.tar.gz"
echo "  tests: $(ls "$OUT"/bin/test_* | wc -l)   libs: $(ls "$OUT"/lib/ | wc -l)   size: $(du -sh "$D/qnx-port-tests-$ARCH.tar.gz" | cut -f1)"
