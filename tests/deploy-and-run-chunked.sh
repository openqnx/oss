#!/bin/bash
# Run the port test suite on a target too small to hold the whole bundle.
#
#   ./tests/deploy-and-run-chunked.sh <arch> <host> [budget-MB]
#
# tests/make-bundle.sh produces ~170 MB, which unpacks to more.  A default
# mkqnximage VM has a 147 MB disk in total, so the ordinary deploy fails
# mid-copy ("scp: Connection closed") and it looks like a network fault.
#
# This ships one batch of tests at a time: for each test binary it resolves the
# shared libraries that binary actually needs, packs tests into batches whose
# *union* of libraries fits the budget, then for each batch copies, runs, and
# deletes before moving on.  Slower than one copy, but it needs only the budget
# free at any moment.
set -o pipefail
WS="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ARCH="$1"; HOST="$2"; BUDGET_MB="${3:-40}"; WITH_OS_RUNTIME="${4:-}"
[ -n "$ARCH" ] && [ -n "$HOST" ] || {
  echo "usage: $0 <arch> <host> [budget-MB] [--with-os-runtime]" >&2; exit 2; }

BUNDLE="$WS/dist/qnx-port-tests-$ARCH"
[ -d "$BUNDLE" ] || { echo "!! no bundle dir: $BUNDLE (run tests/make-bundle.sh)" >&2; exit 1; }

source "$WS/env.sh" "$ARCH" >/dev/null || exit 1
READELF="$QNX_HOST/usr/bin/nto${QNX_CPU}-readelf"
# Target account; a stock image has qnxuser, a mkqnximage VM with --ssh-ident
# only lets root in with the key.
TUSER="${TARGET_USER:-qnxuser}"
source "$WS/tests/ssh-opts.sh"
mapfile -t SSHOPT < <(qnxport_ssh_opts "$WS")
REMOTE=qnx-port-chunk

need() { "$READELF" -d "$1" 2>/dev/null | sed -n 's/.*NEEDED.*\[\(.*\)\]/\1/p'; }

# Resolve one binary's transitive library closure within the bundle, following
# symlink chains the way make-bundle.sh does -- a dangling middle link is a
# run-time "Could not load library", not a copy error.
# Things a test loads at run time, which a DT_NEEDED walk cannot see.  Getting
# this wrong does not look like a missing file -- it looks like a failed port:
# test_ltdl dlopens libz, test_gstreamer loads its plugins through
# GST_PLUGIN_PATH, test_enchant dlopens its aspell provider.
extras_for() {
  case "$1" in
    test_ltdl)      printf 'lib/libz.so lib/libz.so.1 lib/libz.so.1.3.1' ;;
    test_gstreamer|test_gst_plugins_good) printf 'lib/gstreamer-1.0 libexec/gstreamer-1.0' ;;
    test_sdl2)      printf 'lib/libSDL3.so.0' ;;
    test_enchant)   printf 'lib/enchant-2' ;;
    test_python)    printf 'lib/python3.14' ;;
    test_tflite)    printf 'opt/tensorflow-lite/lib' ;;
    test_torch)     printf 'opt/pytorch/lib' ;;
    test_ros2)      printf 'opt/ros/jazzy/lib opt/ros/jazzy/share' ;;
  esac
}

declare -A CLOSURE_CACHE
closure() {
  local start="$1"
  [ -n "${CLOSURE_CACHE[$start]}" ] && { printf '%s' "${CLOSURE_CACHE[$start]}"; return; }
  local -A seen=() ; local queue="$start" next f n p t out=""
  while [ -n "$queue" ]; do
    next=""
    for f in $queue; do
      for n in $(need "$f"); do
        [ -n "${seen[$n]}" ] && continue
        [ -e "$BUNDLE/lib/$n" ] || continue
        seen[$n]=1
        p="$BUNDLE/lib/$n"
        while [ -L "$p" ]; do
          out="$out lib/$(basename "$p")"
          t=$(readlink "$p"); case "$t" in /*) p="$t";; *) p="$(dirname "$p")/$t";; esac
        done
        out="$out lib/$(basename "$p")"
        next="$next $p"
      done
    done
    queue="$next"
  done
  CLOSURE_CACHE[$start]="$out"
  printf '%s' "$out"
}

# An extra brings its own libraries: a GStreamer plugin links its codec
# library, SDL3 links what it was built with.  Shipping the plugin directory
# alone gave "Failed to load plugin ...: Library cannot be found" for every
# plugin whose codec library no test happened to need.
extra_files() {
  local e="$1" p t f out=""
  if [ -d "$BUNDLE/$e" ]; then
    out="$e"
    while IFS= read -r f; do out="$out $(closure "$f")"; done \
      < <(find "$BUNDLE/$e" -type f \( -name '*.so*' -o -perm -u+x \))
  else
    p="$BUNDLE/$e"
    while [ -L "$p" ]; do
      out="$out ${p#$BUNDLE/}"
      t=$(readlink "$p"); case "$t" in /*) p="$t";; *) p="$(dirname "$p")/$t";; esac
    done
    out="$out ${p#$BUNDLE/} $(closure "$p")"
  fi
  printf '%s' "$out"
}

size_of() { # total bytes of a list of bundle-relative paths, deduplicated
  local -A u=(); local p total=0
  for p in $1; do u[$p]=1; done
  for p in "${!u[@]}"; do total=$(( total + $(stat -Lc%s "$BUNDLE/$p" 2>/dev/null || echo 0) )); done
  echo "$total"
}

BUDGET=$(( BUDGET_MB * 1024 * 1024 ))
# Keep every batch's output: the summary alone cannot say why a test failed.
TRANSCRIPT="$WS/logs/ontarget-chunked.$ARCH.txt"; : > "$TRANSCRIPT"
TESTS=$(cd "$BUNDLE/bin" && ls test_* 2>/dev/null | sort)
TOTAL=$(printf '%s\n' $TESTS | wc -l)
echo "== $ARCH -> $TUSER@$HOST   $TOTAL tests, ${BUDGET_MB} MB per batch"

# --with-os-runtime carries QNX's OWN split-libc shared libraries -- libsocket,
# libeventfd, libepoll, libsignalfd, libtimerfd, libfsnotify, libregex, libm --
# out of the SDP and into each batch.  A normal QNX 8 image already has them and
# must not be given copies; a stripped mkqnximage build ships none of them, and
# then every binary this workspace produces dies at load with
# "Could not load library libeventfd.so.1" before main() runs.
#
# These are pieces of QNX's C runtime, not ported open source: there is no
# upstream to build them from, and scripts/check-sdp-leak does not count them
# because this workspace provides no competing copy.  It is still a change in
# what a passing run proves, so it is opt-in and never the default.
OSRT=""
if [ "$WITH_OS_RUNTIME" = "--with-os-runtime" ]; then
  mkdir -p "$BUNDLE/oslib"
  for l in libsocket libeventfd libepoll libsignalfd libtimerfd libfsnotify libregex libm; do
    for d in "$QNX_TARGET_SDP/$QNX_ARCH/lib" "$QNX_TARGET_SDP/$QNX_ARCH/usr/lib"; do
      for f in "$d/$l".so.[0-9]*; do
        [ -e "$f" ] && cp -n "$f" "$BUNDLE/oslib/" 2>/dev/null
      done
    done
  done
  OSRT="oslib"
  echo "-- carrying QNX's split-libc runtime from the SDP ($(ls "$BUNDLE/oslib" | wc -l) libraries)"
fi

# Runtime data some tests read from disk.  Every batch carries it, so it has to
# be small in FILE COUNT as well as bytes: the full terminfo database is 2914
# tiny files, and unpacking it into a 56 MB QNX partition fills the device
# ("tar: No space left on device") before a single test runs.  The tests name
# only a handful of terminals, so a trimmed copy is built once and used instead.
TI="$BUNDLE/terminfo-min"
if [ -d "$BUNDLE/share/terminfo" ]; then
  rm -rf "$TI"; mkdir -p "$TI/share/terminfo"
  for t in xterm xterm-256color dumb ansi vt100 vt220 linux screen; do
    i=$(printf %s "$t" | cut -c1)
    [ -e "$BUNDLE/share/terminfo/$i/$t" ] || continue
    mkdir -p "$TI/share/terminfo/$i"
    cp -L "$BUNDLE/share/terminfo/$i/$t" "$TI/share/terminfo/$i/" 2>/dev/null
  done
  echo "-- terminfo trimmed to $(find "$TI" -type f | wc -l) entries (of $(find "$BUNDLE/share/terminfo" | wc -l) files)"
fi

DATA=""
for d in share/X11/xkb etc/fonts share/fontconfig share/libthai; do
  [ -d "$BUNDLE/$d" ] && DATA="$DATA $d"
done

pass=0; fail=0; failing=""; batch=0
batch_tests=""; batch_files=""

flush() {
  [ -n "$batch_tests" ] || return 0
  batch=$((batch+1))
  local list; list=$(printf '%s\n' $batch_tests | tr '\n' ' ')
  # Every scratch file is named for the architecture: the two targets are
  # usually run at the same time, and shared .chunk-out/.chunk-err files let
  # one run delete the other's results ("batch 1 produced no results").
  local plain="$WS/dist/.chunk-$ARCH.tar"
  # Two steps, because a gzipped archive cannot be appended to: the trimmed
  # terminfo lives outside the bundle and has to come in under its own -C.
  # shellcheck disable=SC2086
  tar cf "$plain" -C "$BUNDLE" $DATA $OSRT $(printf 'bin/%s\n' $batch_tests) \
      bin/run-on-target.sh $(printf '%s\n' $batch_files | tr ' ' '\n' | sort -u) 2>/dev/null
  [ -d "$TI" ] && tar rf "$plain" -C "$TI" share 2>/dev/null
  local mb; mb=$(( $(stat -c%s "$plain") / 1024 / 1024 ))
  printf -- "-- batch %d: %d test(s), %d MB\n" "$batch" "$(printf '%s\n' $batch_tests | wc -w)" "$mb"

  # Stream the archive straight into tar on the target rather than scp'ing it
  # and unpacking in a second step.  On a 61 MB partition the tarball and the
  # tree it unpacks to cannot both exist: that is what "tar: No space left on
  # device" was, not an oversized batch.  Piped in, only the tree ever lands.
  # Run through the suite's own runner, not a reimplementation of it.  Each
  # batch carries bin/run-on-target.sh but NOT bin/tests.list, and the runner
  # falls back to "every test_* next to me" -- so it runs exactly this batch,
  # with the environment it already knows how to set up (XDG_RUNTIME_DIR for
  # Wayland, GST_PLUGIN_PATH and the plugin scanner for GStreamer, TERMINFO,
  # FONTCONFIG_PATH, XKB_CONFIG_ROOT).  Reimplementing that inline is how
  # test_wayland and test_gstreamer came back "failing" when they were fine.
  gzip -c "$plain" | ssh "${SSHOPT[@]}" "$TUSER@$HOST" "
    rm -rf \$HOME/$REMOTE && mkdir -p \$HOME/$REMOTE &&
    tar xzf - -C \$HOME/$REMOTE &&
    cd \$HOME/$REMOTE &&
    { [ -d oslib ] && LD_LIBRARY_PATH=\$PWD/oslib && export LD_LIBRARY_PATH; }
    ./bin/run-on-target.sh
    rc=\$?
    cd \$HOME && rm -rf \$HOME/$REMOTE
    exit \$rc
  " > "$WS/dist/.chunk-$ARCH.out" 2>"$WS/dist/.chunk-$ARCH.err"
  rm -f "$plain"
  # Only lines that start at column 0 are per-test verdicts.  run-on-target.sh
  # also echoes each failing test's own output indented, and those lines begin
  # with "FAIL <check name>" too -- reading them as verdicts turns one failed
  # test into eight failures and loses the test name.
  while read -r st name _; do
    case "$st" in
      PASS) pass=$((pass+1));;
      FAIL) fail=$((fail+1)); failing="$failing $name";;
    esac
  done < <(grep -E '^(PASS|FAIL) ' "$WS/dist/.chunk-$ARCH.out" 2>/dev/null)
  cat "$WS/dist/.chunk-$ARCH.out" >> "$TRANSCRIPT"
  if ! grep -q "^\(PASS\|FAIL\) " "$WS/dist/.chunk-$ARCH.out" 2>/dev/null; then
    echo "!! batch $batch produced no results; target said:"; head -5 "$WS/dist/.chunk-$ARCH.err"
  fi
  rm -f "$WS/dist/.chunk-$ARCH.out" "$WS/dist/.chunk-$ARCH.err"
  batch_tests=""; batch_files=""
}

skipped=0; skipping=""
for t in $TESTS; do
  c=$(closure "$BUNDLE/bin/$t")
  own=$(size_of "$c")
  # A test whose own library closure is already over budget cannot be made to
  # fit by batching -- OpenCV, PCL and ONNX Runtime each pull in about 100 MB,
  # more than this kind of target has free in total.  Say so rather than
  # shipping it, failing on ENOSPC, and calling that a failed port.
  if [ "$own" -gt "$BUDGET" ]; then
    skipped=$((skipped+1)); skipping="$skipping $t($(( own / 1024 / 1024 ))MB)"
    continue
  fi
  cand="$batch_files $c"
  if [ -n "$batch_tests" ] && [ "$(size_of "$cand")" -gt "$BUDGET" ]; then
    flush || exit 1
    cand="$c"
  fi
  batch_tests="$batch_tests $t"
  local_extra=$(extras_for "$t")
  for e in $local_extra; do [ -e "$BUNDLE/$e" ] && cand="$cand $(extra_files "$e")"; done
  batch_files="$cand"
done
flush || exit 1

echo "--------------------------------------------"
echo "passed: $pass   failed: $fail   not run: $skipped"
echo "full output: $TRANSCRIPT"
[ $skipped -eq 0 ] || {
  echo "too large for this target (closure exceeds the ${BUDGET_MB} MB budget):"
  printf '  %s\n' $skipping
}
[ $fail -eq 0 ] || { echo "failing:$failing"; exit 1; }
