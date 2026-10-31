#!/bin/sh
# OpenSSH: ssh-keygen makes ed25519 and RSA keys, fingerprints them (the
# private and public halves agree), re-derives a public key, and signs and
# verifies a file (ssh-keygen -Y); then sshd runs on loopback with a private
# config and ssh runs a command over it with key authentication, and scp
# copies a file (SFTP subsystem) -- unless this system lacks sshd's
# privilege-separation user or directory, which is reported.
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
N=openssh
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
KG=$L/bin/ssh-keygen
to() { if command -v timeout >/dev/null 2>&1; then timeout "$@"; else shift; "$@"; fi; }
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
info "$("$L/bin/ssh" -V 2>&1)"
"$KG" -q -t ed25519 -N '' -C qnx-test -f id_ed25519 && ok "ssh-keygen -t ed25519" || bad "ed25519 keygen"
fp1=$("$KG" -l -f id_ed25519.pub 2>&1); fp2=$("$KG" -l -f id_ed25519 2>&1)
case "$fp1" in "256 SHA256:"*"qnx-test (ED25519)") ok "fingerprint: $(echo $fp1 | cut -d' ' -f2)" ;; *) bad "fingerprint: $fp1" ;; esac
[ "$fp1" = "$fp2" ] && ok "private and public key fingerprints agree" || bad "fingerprints differ: $fp2"
"$KG" -y -f id_ed25519 > derived.pub 2>&1 && [ "$(cut -d' ' -f1-2 derived.pub)" = "$(cut -d' ' -f1-2 id_ed25519.pub)" ] && ok "ssh-keygen -y re-derives the public key" || bad "-y"
"$KG" -q -t rsa -b 2048 -N '' -C rsa-test -f id_rsa && case "$("$KG" -l -f id_rsa.pub)" in "2048 SHA256:"*"(RSA)") true ;; *) false ;; esac && ok "ssh-keygen -t rsa -b 2048" || bad "rsa keygen"
echo "QNX signed payload" > msg.txt
"$KG" -Y sign -f id_ed25519 -n file msg.txt > /dev/null 2>&1 && [ -s msg.txt.sig ] && ok "-Y sign: msg.txt.sig" || bad "-Y sign"
printf 'qnx@test %s\n' "$(cut -d' ' -f1-2 id_ed25519.pub)" > allowed
"$KG" -Y verify -f allowed -I qnx@test -n file -s msg.txt.sig < msg.txt > v.out 2>&1 && ok "-Y verify: good signature" || { bad "-Y verify"; cat v.out; }
echo "tampered" > bad.txt
"$KG" -Y verify -f allowed -I qnx@test -n file -s msg.txt.sig < bad.txt > /dev/null 2>&1 && bad "-Y verify accepted a tampered file" || ok "-Y verify rejects a tampered file"
# --- sshd + ssh on loopback ---
P=$((22000 + $$ % 3000)); U=$(id -un 2>/dev/null || echo root)
"$KG" -q -t ed25519 -N '' -f hostkey; cp id_ed25519.pub authkeys; chmod 600 hostkey authkeys
cat > sshd_config <<EOF
Port $P
ListenAddress 127.0.0.1
HostKey $T/hostkey
PidFile none
AuthorizedKeysFile $T/authkeys
StrictModes no
PasswordAuthentication no
KbdInteractiveAuthentication no
PermitRootLogin yes
UseDNS no
PerSourcePenalties no
SshdSessionPath $L/libexec/sshd-session
SshdAuthPath $L/libexec/sshd-auth
Subsystem sftp $L/libexec/sftp-server
LogLevel ERROR
EOF
SSHD=$L/sbin/sshd
t=$("$SSHD" -t -f sshd_config 2>&1); rc=$?
if [ $rc -ne 0 ]; then
  case "$t" in
    *"Privilege separation user"*|*"Missing privilege separation directory"*)
      info "sshd not run on loopback: $t" ;;
    *) bad "sshd -t: $t" ;;
  esac
else
  ok "sshd -t: configuration valid"
  "$SSHD" -D -e -f "$T/sshd_config" > sshd.log 2>&1 &
  S=$!; track $S
  o="-F /dev/null -o BatchMode=yes -o StrictHostKeyChecking=no -o UserKnownHostsFile=$T/known_hosts -o GlobalKnownHostsFile=/dev/null -o IdentitiesOnly=yes -o ConnectTimeout=10 -i $T/id_ed25519"
  i=0; while [ $i -lt 15 ]; do r=$(to 30 "$L/bin/ssh" $o -p $P "$U@127.0.0.1" 'echo "remote:$((6*7))"' 2> ssh.err) && break; sleep 1; i=$((i+1)); done
  [ "$r" = remote:42 ] && ok "ssh to 127.0.0.1:$P ran a command (key auth): $r" || { bad "ssh command: $r"; tail -3 ssh.err; tail -5 sshd.log; }
  awk '/127\.0\.0\.1/ && /ssh-ed25519/ {f=1} END {exit !f}' known_hosts 2>/dev/null && ok "host key recorded in known_hosts" || bad "known_hosts"
  to 60 "$L/bin/scp" -q $o -P $P msg.txt "$U@127.0.0.1:$T/copied.txt" 2> scp.err && cmp -s msg.txt copied.txt && ok "scp (SFTP) copied a file" || { bad "scp"; tail -3 scp.err; }
  kill $S 2>/dev/null; wait $S 2>/dev/null
fi
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
