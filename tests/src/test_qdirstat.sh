#!/bin/sh
# QDirStat on Qt's offscreen platform (no display). A directory tree with
# known files is scanned by qdirstat-cache-writer (the cache lists them with
# their sizes); that cache, gzipped into the tree as .qdirstat.cache.gz,
# must then be found and used by QDirStat itself while it reads the tree --
# which only happens when the directory entries' full names are read (the
# d_name fix from qnx-ports, ports/qdirstat/patches/0001) -- and reading
# must finish. Then QDirStat reads the plain cache file given with --cache.
# QDirStat's log (/tmp/qdirstat-<user>/qdirstat.log) is what is checked.
echo "== qdirstat =="
QD=/usr/local/bin/qdirstat; [ -x "$QD" ] || QD=qdirstat
CW=/usr/local/bin/qdirstat-cache-writer; [ -x "$CW" ] || CW=$(command -v qdirstat-cache-writer)
PERL=/usr/local/bin/perl; [ -x "$PERL" ] || PERL=perl
T=$(mktemp -d "${TMPDIR:-/tmp}/qnxport-qdirstat.XXXXXX") || { echo "FAIL qdirstat (no temporary directory)"; exit 1; }
# The real path: qdirstat-cache-writer records the directory resolved
# through symbolic links (Cwd::abs_path), QDirStat compares it with the path
# it reads unresolved -- and /tmp is a link to /data/var/tmp on QNX images.
# Under a linked path the cache would be found but not used.
T=$("$PERL" -MCwd -e 'print Cwd::abs_path($ARGV[0])' "$T") || exit 1
cd "$T" || exit 1
fails=0
ok() { echo "  ok   $1"; }
bad() { echo "  FAIL $1"; fails=$((fails + 1)); }
chk() { if [ "$1" = 0 ]; then ok "$2"; else bad "$2"; fi; }
mkf() { "$PERL" -e 'open(F, ">", $ARGV[0]) or die; print F "q" x $ARGV[1]; close F' "$1" "$2"; }
export HOME="$T" XDG_CONFIG_HOME="$T/.config" QT_QPA_PLATFORM=offscreen
LOG=

mkdir -p tree/sub/deeper
mkf tree/a.txt 1000; mkf tree/sub/b.bin 2000; mkf "tree/sub/c d.bin" 4096; mkf tree/sub/deeper/long-file-name-qnx.dat 123

# (paths are matched by their tail: mktemp's directory can be reached
# through a symbolic link, /tmp -> /data/var/tmp, which the writer resolves)
# ---- the cache writer ----
"$PERL" "$CW" -l "$T/tree/sub" "$T/sub.cache" > cw.out 2>&1; rc=$?
[ $rc -ne 0 ] && sed 's/^/  info /' cw.out
chk $rc "qdirstat-cache-writer ran"
head -1 sub.cache 2>/dev/null | grep -q '^\[qdirstat 2.0 cache file\]'; chk $? "cache file header: qdirstat 2.0 cache file"
grep -q "^F[[:space:]].*/tree/sub/b\.bin[[:space:]]*2000[[:space:]]" sub.cache; chk $? "cache lists b.bin with 2000 bytes"
grep -q "^F[[:space:]].*/tree/sub/c%20d\.bin[[:space:]]*4096[[:space:]]" sub.cache; chk $? "cache lists 'c d.bin' (escaped c%20d.bin) with 4096 bytes"
grep -q "long-file-name-qnx.dat[[:space:]]*123[[:space:]]" sub.cache; chk $? "cache lists the file in deeper/ with 123 bytes"
"$PERL" -MIO::Compress::Gzip=gzip -e 'gzip $ARGV[0] => $ARGV[1] or exit 1' sub.cache tree/sub/.qdirstat.cache.gz
chk $? "cache gzipped into the tree as .qdirstat.cache.gz"

# QDirStat prints where it logs (/tmp/qdirstat-<user>/qdirstat.log, the
# previous log rotated away) on its standard output: find_log <output>
find_log() {
  i=0
  while [ $i -lt 30 ]; do
    LOG=$(sed -n 's/.*Logging to \(.*qdirstat\.log\).*/\1/p' "$1" 2>/dev/null | head -1)
    [ -n "$LOG" ] && return 0
    sleep 1; i=$((i + 1))
  done
  return 1
}
# wait_log <pattern> <seconds> <pid>
wait_log() {
  i=0
  while [ $i -lt "$2" ]; do
    grep -q "$1" "$LOG" 2>/dev/null && return 0
    kill -0 "$3" 2>/dev/null || { grep -q "$1" "$LOG" 2>/dev/null; return $?; }
    sleep 1; i=$((i + 1))
  done
  return 1
}
qd_stop() { kill "$1" 2>/dev/null; sleep 1; kill -9 "$1" 2>/dev/null; wait "$1" 2>/dev/null; }

# ---- QDirStat reads the tree ----
"$QD" "$T/tree" > qd1.out 2>&1 &
pid=$!
find_log qd1.out; chk $? "QDirStat started and logs to ${LOG:-?}"
wait_log "Reading finished after" 90 $pid; rc=$?
chk $rc "QDirStat read the tree to the end (offscreen)"
[ $rc -ne 0 ] && { tail -5 qd1.out | sed 's/^/  info /'; tail -5 "$LOG" 2>/dev/null | sed 's/^/  info log: /'; }
grep -q "Found cache file \.qdirstat\.cache\.gz" "$LOG" 2>/dev/null
chk $? "found .qdirstat.cache.gz while reading tree/sub (full directory entry names)"
grep -q "Using cache file $T/tree/sub/\.qdirstat\.cache\.gz for $T/tree/sub" "$LOG" 2>/dev/null
chk $? "used it for tree/sub (the cache's top directory matches)"
grep -q -E "Syntax error|Too many" "$LOG" 2>/dev/null; [ $? -ne 0 ]; chk $? "the cache file parsed without errors"
grep -E 'mount|Mount' "$LOG" 2>/dev/null | head -3 | sed 's/^.*\] /  info /'
qd_stop $pid

# ---- QDirStat reads a cache file ----
"$QD" --cache "$T/sub.cache" > qd2.out 2>&1 &
pid=$!
LOG=; find_log qd2.out
wait_log "Reading finished after" 90 $pid; rc=$?
chk $rc "QDirStat --cache sub.cache: reading finished"
grep -q "Reading cache file .*/sub\.cache" "$LOG" 2>/dev/null; chk $? "the cache file named on the command line was read"
qd_stop $pid

cd /; rm -rf "$T"
[ $fails -eq 0 ] && { echo "PASS qdirstat"; exit 0; }
echo "FAIL qdirstat ($fails check(s) failed)"; exit 1
