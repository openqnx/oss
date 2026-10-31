#!/bin/sh
# sudo: visudo -c checks a sudoers file (a good one passes, a broken one is
# rejected with the line number), cvtsudoers converts it to JSON and to LDIF
# with the rules intact, and sudo -V reports its version and plugin. If the
# system's sudoers lets the running user in, sudo -n runs a command too.
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
N=sudo
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
has() { awk -v p="$1" 'index($0, p) {f=1} END {exit !f}'; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
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
r=$("$L/bin/sudo" -V 2>&1)
v=$(echo "$r" | awk '/^Sudo version / {print; exit}')
[ -n "$v" ] && ok "sudo -V: $v" || bad "sudo -V: $(echo "$r" | head -n 2)"
echo "$r" | awk '/setrlimit/ {f=1} END {exit !f}' && bad "sudo warns: $(echo "$r" | awk '/setrlimit/ {print; exit}')"
[ -f "$L/etc/sudoers" ] && ok "sudoers installed at $L/etc/sudoers" || bad "no $L/etc/sudoers (sudo reads its configuration from there)"
cat > good <<'EOF'
Defaults env_reset
Cmnd_Alias QNXTOOLS = /usr/bin/pidin, /bin/slay
User_Alias OPS = alice, bob
root ALL=(ALL:ALL) ALL
OPS ALL=(root) NOPASSWD: QNXTOOLS
%wheel ALL=(ALL) ALL
EOF
printf 'root ALL=(ALL ALL\nbroken line here\n' > bad
chmod 0440 good bad
r=$("$L/sbin/visudo" -c -f good 2>&1); rc=$?
[ $rc -eq 0 ] && echo "$r" | has "parsed OK" && ok "visudo -c: good sudoers parsed OK" || bad "visudo -c good (rc=$rc): $r"
r=$("$L/sbin/visudo" -c -f bad 2>&1); rc=$?
[ $rc -ne 0 ] && echo "$r" | has "syntax error" && ok "visudo -c: broken sudoers rejected ($(echo "$r" | head -n 1 | cut -c1-60))" || bad "visudo -c bad (rc=$rc): $r"
"$L/bin/cvtsudoers" -f json -o good.json good > cj.log 2>&1 && ok "cvtsudoers -f json" || { bad "cvtsudoers json"; head -3 cj.log; }
has '"User_Specs"' < good.json && has '"/usr/bin/pidin"' < good.json && has '"alice"' < good.json && ok "JSON holds User_Specs, the command alias and the users" || bad "JSON content"
"$L/bin/cvtsudoers" -f ldif -b ou=SUDOers,dc=qnx,dc=test -o good.ldif good > cl.log 2>&1 && has "sudoUser: root" < good.ldif && has "objectClass: sudoRole" < good.ldif \
  && ok "cvtsudoers -f ldif: sudoRole entries" || { bad "cvtsudoers ldif"; head -3 cl.log; }
r=$("$L/bin/cvtsudoers" -f sudoers -m user=bob good 2>&1)
echo "$r" | has "NOPASSWD" && ! echo "$r" | has "%wheel" && ok "cvtsudoers -m user=bob filters to bob's rules" || bad "cvtsudoers -m: $r"
r=$("$L/bin/sudo" -n id -u 2>&1 < /dev/null)
case "$r" in 0) ok "sudo -n id -u: 0 (the system sudoers allows this user)" ;; *) info "sudo -n not usable here (no /etc/sudoers set up for it?): $(echo "$r" | head -n 1)" ;; esac
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
