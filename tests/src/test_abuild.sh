#!/bin/sh
# abuild (Alpine's package build tools): abuild-tar --hash adds the per-file
# SHA1 PAX records apk expects and --cut drops the end-of-archive blocks;
# abuild-gzsplit splits a concatenated .apk-style gzip stream into its
# control and data parts; newapkbuild writes an APKBUILD from options; and
# abuild itself (bash) reports its version and the build architecture.
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
N=abuild
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
fhas() { awk -v p="$2" 'index($0, p) {f=1} END {exit !f}' "$1"; }
# Binary files (tar archives): awk stops at a NUL byte on this system, so
# search what strings(1) extracts.
bhas() { strings "$1" 2>/dev/null | awk -v p="$2" 'index($0, p) {f=1} END {exit !f}'; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
B=$L/bin; BASH=$L/bin/bash
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
HOME=$T; export HOME
mkdir d; echo "payload one" > d/one.txt; echo "payload two" > d/two.txt
tar -cf plain.tar d 2>/dev/null
# --hash copies until its input ends and then exits 1 by design (apk's build
# pipes it on): judge it by its output.
"$B/abuild-tar" --hash < plain.tar > hashed.tar 2> h.err; [ "$(wc -c < hashed.tar)" -gt "$(wc -c < plain.tar)" ] && ok "abuild-tar --hash: output grew by the PAX records" || { bad "abuild-tar --hash"; head -3 h.err; }
if bhas hashed.tar "APK-TOOLS.checksum.SHA1="; then ok "PAX record APK-TOOLS.checksum.SHA1 present"
else bad "no APK-TOOLS.checksum.SHA1 record"; strings hashed.tar | head -n 8; fi
bhas hashed.tar "payload one" && bhas hashed.tar "payload two" && ok "file data carried through" || bad "file data missing"
# (A plain ustar input has no PAX headers of its own, so abuild-tar emits
# nameless PAX blocks -- apk reads them, a generic tar -t stops at them.)
"$B/abuild-tar" --cut < plain.tar > cut.tar && [ "$(wc -c < cut.tar)" -lt "$(wc -c < plain.tar)" ] && [ $(( $(wc -c < cut.tar) % 512 )) = 0 ] \
  && ok "abuild-tar --cut: end-of-archive removed ($(wc -c < plain.tar | tr -d ' ') -> $(wc -c < cut.tar | tr -d ' ') bytes)" || bad "abuild-tar --cut"
mkdir c; printf 'pkgname = qnxdemo\npkgver = 1.0-r0\n' > c/.PKGINFO
( cd c && tar -cf ../control.tar .PKGINFO ) 2>/dev/null; tar -cf data.tar d 2>/dev/null
gzip -c control.tar > control.gz; gzip -c data.tar > data.gz; cat control.gz data.gz > pkg.apk
mkdir s; ( cd s && "$B/abuild-gzsplit" < ../pkg.apk ) 2> s.err && cmp -s s/control.tar.gz control.gz && cmp -s s/data.tar.gz data.gz \
  && ok "abuild-gzsplit: control.tar.gz and data.tar.gz split exactly" || { bad "abuild-gzsplit"; ls s; head -3 s.err; }
A=$(apk --print-arch 2>/dev/null); [ -n "$A" ] || case "$(uname -m)" in aarch64*|*arm*|QEMU_virt) A=aarch64 ;; *) A=x86_64 ;; esac
CBUILD=$A; export CBUILD
if [ -x "$BASH" ]; then
  r=$("$BASH" "$B/abuild" -V 2>&1); case "$r" in "abuild "[0-9]*) ok "abuild -V: $r" ;; *) bad "abuild -V: $r" ;; esac
  r=$("$BASH" "$B/abuild" -A 2>&1); [ "$r" = "$A" ] && ok "abuild -A: $r" || bad "abuild -A: $r (want $A)"
  "$BASH" "$B/newapkbuild" -a -d "QNX demo package" -l MIT -u https://example.invalid/qnxdemo qnxdemo-1.2.3 > n.log 2>&1
  f=qnxdemo/APKBUILD
  if [ -f $f ]; then
    m=0; for k in 'pkgname=qnxdemo' 'pkgver=1.2.3' 'pkgdesc="QNX demo package"' 'license="MIT"' 'url="https://example.invalid/qnxdemo"' 'make DESTDIR="$pkgdir" install'; do fhas $f "$k" || { m=$((m+1)); info "missing: $k"; }; done
    [ $m -eq 0 ] && ok "newapkbuild -a: APKBUILD with name, version, description, license, url, autotools package()" || bad "APKBUILD content ($m missing)"
  else bad "newapkbuild wrote no APKBUILD"; head -5 n.log; fi
else bad "bash not installed (abuild and newapkbuild are bash scripts)"; fi
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
