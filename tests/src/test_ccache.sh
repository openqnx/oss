#!/bin/sh
# ccache: QNX targets have no compiler of their own (a vanilla image ships
# none; the llvm port's clang is optional), so ccache wraps a fake compiler
# script named gcc that "preprocesses" (-E) and "compiles" (-c, writing a
# checksum as the object) and counts its real runs. Checked: a first
# compile is a miss and runs the compiler, a repeat is a direct-mode hit
# (compiler not run, same object), other options or a changed source miss,
# preprocessor mode hits (CCACHE_NODIRECT), the statistics count each
# (--print-stats), the cache directory holds the result files, -o writes
# ccache.conf and --get-config reads it, -x shows compression, -z zeroes
# and -C clears, --hash-file is content-based and both --hash-file and
# --checksum-file give known values, CCACHE_DISABLE bypasses.
# Run the test itself in the background, its output to a log, and wait at
# most 170 s: nothing it leaves behind can hold the runner's output, and a
# hang ends as a FAIL with whatever the test printed so far.
if [ -z "$QNXPORT_INNER" ]; then
  QNXPORT_INNER=/tmp/qnxport-out.$$; export QNXPORT_INNER
  sh "$0" "$@" > "$QNXPORT_INNER" 2>&1 < /dev/null &
  qp=$!; qi=0
  while kill -0 $qp 2>/dev/null && [ $qi -lt 170 ]; do sleep 1; qi=$((qi+1)); done
  if kill -0 $qp 2>/dev/null; then
    for p in $(cat "$QNXPORT_INNER.pids" 2>/dev/null); do kill $p 2>/dev/null; done
    kill $qp 2>/dev/null; sleep 1; kill -KILL $qp 2>/dev/null
    cat "$QNXPORT_INNER"; echo "  FAIL watchdog: still running after 170 s -- stopped"
    echo "FAIL ${0##*/} (watchdog)"; rm -f "$QNXPORT_INNER" "$QNXPORT_INNER.pids"; exit 1
  fi
  wait $qp; rc=$?; cat "$QNXPORT_INNER"; rm -f "$QNXPORT_INNER" "$QNXPORT_INNER.pids"; exit $rc
fi
N=ccache
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
has() { awk -v p="$1" 'index($0, p) {f=1} END {exit !f}'; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
D=$(cd "$(dirname "$0")" && pwd)   # helpers next to this test
T=$(mktemp -d /tmp/qnxport-$N.XXXXXX) && cd "$T" || { echo "FAIL $N (no temp dir)"; exit 1; }
# Whatever the test starts stops with it, on every exit path (track PID;
# qstop_hook for anything else); the outer shell above kills them on a hang.
QP=$QNXPORT_INNER.pids; : > "$QP"
track() { echo "$1" >> "$QP"; }
qstop_hook() { :; }
stopall() {
  qstop_hook 2>/dev/null
  for p in $(cat "$QP" 2>/dev/null); do kill "$p" 2>/dev/null; done
  : > "$QP"; [ -n "$T" ] && [ -d "$T" ] && { cd /; rm -rf "$T"; }
}
trap stopall EXIT
trap 'exit 143' TERM
to() { if command -v timeout >/dev/null 2>&1; then timeout "$@"; else shift; "$@"; fi; }
CC_=$L/bin/ccache
r=$("$CC_" --version 2>&1 | head -n 1); [ "$r" = "ccache version 4.12.1" ] && ok "ccache --version: 4.12.1" || bad "version: $r"
# The fake compiler: "gcc" by name (ccache recognises the compiler type from
# it). -E prints the source with a line marker, as a preprocessor would;
# ccache runs it as "gcc -E -o <tmp>.i a.c"; -c writes an "object" (the
# cksum of the source) to -o's file and counts
# the real compilations in calls.log.
mkdir bin
cat > bin/gcc <<'EOC'
#!/bin/sh
mode=link; out=; src=
while [ $# -gt 0 ]; do
  case "$1" in
    -E) mode=pp ;;
    -c) mode=cc ;;
    -o) out=$2; shift ;;
    -*) ;;
    *) src=$1 ;;
  esac; shift
done
case $mode in
  pp) { echo "# 1 \"$src\""; cat "$src"; } > "${out:-/dev/stdout}" ;;
  cc) echo "cc $src" >> "$FAKECC_LOG"; { echo "FAKEOBJ"; cksum < "$src"; } > "$out" ;;
  *) echo "fake gcc: only -E and -c" >&2; exit 1 ;;
esac
EOC
chmod 755 bin/gcc
FAKECC_LOG=$T/calls.log; export FAKECC_LOG
CCACHE_DIR=$T/cache; CCACHE_TEMPDIR=$T/cache/tmp; export CCACHE_DIR CCACHE_TEMPDIR
unset CCACHE_DISABLE CCACHE_PREFIX CCACHE_CONFIGPATH
calls() { wc -l < "$FAKECC_LOG" 2>/dev/null | tr -d ' '; }
stat_() { "$CC_" --print-stats | awk -v k="$1" '$1 == k {print $2}'; }
printf 'int answer(void) { return 42; }\n' > a.c
r=$("$CC_" "$T/bin/gcc" -O2 -c a.c -o a.o 2>&1); rc=$?
[ $rc -eq 0 ] && [ "$(calls)" = 1 ] && [ "$(head -n 1 a.o)" = FAKEOBJ ] && ok "first compile: a miss, the compiler ran once" || bad "first compile (rc $rc, calls $(calls)): $r"
cp a.o a.o.1; rm a.o
r=$("$CC_" "$T/bin/gcc" -O2 -c a.c -o a.o 2>&1); rc=$?
[ $rc -eq 0 ] && [ "$(calls)" = 1 ] && cmp -s a.o a.o.1 && ok "same compile again: served from the cache (compiler not run), same object" || bad "second compile (rc $rc, calls $(calls)): $r"
[ "$(stat_ direct_cache_hit)" = 1 ] && [ "$(stat_ cache_miss)" = 1 ] && ok "stats: 1 direct hit, 1 miss" || bad "stats: $("$CC_" --print-stats | grep -E 'hit|miss' | tr '\n' ' ')"
r=$("$CC_" "$T/bin/gcc" -O3 -c a.c -o a3.o 2>&1)
[ "$(calls)" = 2 ] && ok "other options (-O3): a miss, the compiler ran" || bad "-O3: calls $(calls) $r"
printf 'int answer(void) { return 43; }\n' > a.c
r=$("$CC_" "$T/bin/gcc" -O2 -c a.c -o a.o 2>&1)
[ "$(calls)" = 3 ] && ! cmp -s a.o a.o.1 && ok "changed source: a miss, a new object" || bad "changed source: calls $(calls) $r"
cp a.o a.o.2
r=$(CCACHE_NODIRECT=1 "$CC_" "$T/bin/gcc" -O2 -c a.c -o a.o 2>&1)
[ "$(calls)" = 3 ] && cmp -s a.o a.o.2 && [ "$(stat_ preprocessed_cache_hit)" = 1 ] && ok "preprocessor mode (CCACHE_NODIRECT): a preprocessed hit, compiler not run" || bad "preprocessed hit: calls $(calls), $(stat_ preprocessed_cache_hit) $r"
n=$(find "$CCACHE_DIR" -type f -name '*R' | wc -l | tr -d ' ')
[ "$n" -ge 3 ] && ok "cache directory: $n result files under \$CCACHE_DIR" || bad "result files: $n"
"$CC_" -o max_files=1234 && [ "$("$CC_" --get-config max_files)" = 1234 ] && grep -q '^max_files = 1234$' "$CCACHE_DIR/ccache.conf" \
  && ok "-o max_files=1234: in \$CCACHE_DIR/ccache.conf, --get-config reads it back" || bad "config: $("$CC_" --get-config max_files)"
r=$("$CC_" -x 2>&1); echo "$r" | has "Total data:" && ok "-x: compression statistics (zstd)" || bad "-x: $r"
"$CC_" -z >/dev/null && [ "$(stat_ direct_cache_hit)" = 0 ] && [ "$(stat_ cache_miss)" = 0 ] && ok "-z: statistics zeroed" || bad "-z"
"$CC_" -C >/dev/null
n=$(find "$CCACHE_DIR" -type f -name '*R' | wc -l | tr -d ' ')
r=$("$CC_" "$T/bin/gcc" -O2 -c a.c -o a.o 2>&1)
[ "$n" = 0 ] && [ "$(calls)" = 4 ] && [ "$(stat_ cache_miss)" = 1 ] && ok "-C: cache cleared, the next compile is a miss" || bad "-C: $n files left, calls $(calls)"
printf 'QNX ccache hash test\n' > h1; cp h1 h2; printf 'QNX ccache hash test!\n' > h3
x1=$("$CC_" --hash-file h1); x2=$("$CC_" --hash-file h2); x3=$("$CC_" --hash-file h3)
[ -n "$x1" ] && [ "$x1" = "$x2" ] && [ "$x1" != "$x3" ] && ok "--hash-file: equal for equal content, different for different" || bad "hash-file: $x1 $x2 $x3"
# Known answers (BLAKE3-based ccache hash; XXH3-128): the same on x86_64
# and aarch64, so neither depends on the byte order or SIMD path.
c1=$("$CC_" --checksum-file h1 2>&1)
[ "$x1" = 04196s842l6tvkk3ami5f7r34nan6rmis ] && [ "$c1" = 70f8eecc611e63a498e00c02cf66bce5 ] \
  && ok "--hash-file / --checksum-file: the known values" || bad "hash-file $x1, checksum-file $c1"
r=$(CCACHE_DISABLE=1 "$CC_" "$T/bin/gcc" -O2 -c a.c -o a.o 2>&1)
[ "$(calls)" = 5 ] && ok "CCACHE_DISABLE: the compiler runs directly" || bad "CCACHE_DISABLE: calls $(calls)"
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
