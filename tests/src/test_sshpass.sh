#!/bin/sh
# sshpass 1.10 without a network: sshpass runs a program on a pseudo-terminal
# of its own and answers its password prompt. The program here is a small
# script that prompts on its controlling terminal (/dev/tty) the way ssh
# does and says on stdout whether the password was right. Checks: the
# password from -p, -f (file), -e (SSHPASS) and -d (descriptor) is accepted;
# a wrong one makes the program prompt again and sshpass exits 5 ("incorrect
# password"); -P matches a custom prompt; an unknown host key question makes
# sshpass exit 6; the program's exit status is passed through; -V. (A real
# ssh session needs a password-authenticated account on the target, which a
# test image does not have: not exercised.)
echo "== sshpass =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
T=$(mktemp -d /tmp/qnxport-sshpass.XXXXXX) || { echo "FAIL sshpass (mktemp)"; exit 1; }
trap 'cd /; rm -rf "$T"' EXIT
trap 'echo "  FAIL interrupted"; exit 1' INT TERM HUP
cd "$T" || exit 1

# fake-ssh <prompt> <password> [hostkey]: prompts up to 3 times on /dev/tty
cat > fake-ssh <<'SH'
#!/bin/sh
prompt=$1; want=$2
if [ "$3" = hostkey ]; then
  printf "The authenticity of host '127.0.0.1 (127.0.0.1)' can't be established.\nAre you sure you want to continue connecting (yes/no)? " > /dev/tty
  read -r ans < /dev/tty; echo "answered: $ans"; exit 9
fi
n=0
while [ $n -lt 3 ]; do
  printf '%s' "$prompt" > /dev/tty
  IFS= read -r pw < /dev/tty || exit 8
  if [ "$pw" = "$want" ]; then echo "accepted after $n failure(s)"; exit 0; fi
  printf '\nPermission denied, please try again.\n' > /dev/tty
  n=$((n+1))
done
echo "rejected"; exit 7
SH
chmod +x fake-ssh
PW='S3cret pass!'
TO=20   # seconds before a hung check is abandoned

# run with a watchdog: sshpass waiting on a prompt that never matches
# must not hang the suite.
run() {
  "$@" > out.txt 2> err.txt &
  p=$!; i=0
  while kill -0 $p 2>/dev/null && [ $i -lt $TO ]; do sleep 1; i=$((i+1)); done
  if kill -0 $p 2>/dev/null; then kill -9 $p 2>/dev/null; wait $p 2>/dev/null; rc=124; else wait $p; rc=$?; fi
  out=$(cat out.txt)
}

V=$(sshpass -V 2>&1 | head -n 1)
[ "$V" = "sshpass 1.10" ] && ok "sshpass -V: $V" || bad "sshpass -V: $V"

run sshpass -p "$PW" ./fake-ssh "user@host's password: " "$PW"
[ $rc -eq 0 ] && [ "$out" = "accepted after 0 failure(s)" ] && ok "-p: password answered on the pty, accepted" || bad "-p: rc=$rc out='$out' $(cat err.txt)"

printf '%s\n' "$PW" > pw.txt
run sshpass -f pw.txt ./fake-ssh "Password: " "$PW"
[ $rc -eq 0 ] && [ "$out" = "accepted after 0 failure(s)" ] && ok "-f: password from a file" || bad "-f: rc=$rc out='$out' $(cat err.txt)"

SSHPASS=$PW; export SSHPASS
run sshpass -e ./fake-ssh "Password: " "$PW"
unset SSHPASS
[ $rc -eq 0 ] && [ "$out" = "accepted after 0 failure(s)" ] && ok "-e: password from SSHPASS" || bad "-e: rc=$rc out='$out' $(cat err.txt)"

run sh -c "sshpass -d 3 ./fake-ssh 'Password: ' '$PW' 3< pw.txt"
[ $rc -eq 0 ] && [ "$out" = "accepted after 0 failure(s)" ] && ok "-d 3: password from a descriptor" || bad "-d: rc=$rc out='$out' $(cat err.txt)"

run sshpass -p wrong ./fake-ssh "Password: " "$PW"
[ $rc -eq 5 ] && ok "wrong password: prompted again, sshpass exits 5" || bad "wrong password: rc=$rc (want 5) out='$out' $(cat err.txt)"

run sshpass -P "PIN:" -p 4711 ./fake-ssh "Enter PIN: " 4711
[ $rc -eq 0 ] && [ "$out" = "accepted after 0 failure(s)" ] && ok "-P PIN: a custom prompt matched" || bad "-P: rc=$rc out='$out' $(cat err.txt)"

run sshpass -p "$PW" ./fake-ssh "Password: " "$PW" hostkey
[ $rc -eq 6 ] && ok "unknown host key question: sshpass exits 6" || bad "host key: rc=$rc (want 6) out='$out' $(cat err.txt)"

run sshpass -p x sh -c 'exit 3'
[ $rc -eq 3 ] && ok "the program's exit status (3) is passed through" || bad "exit status: rc=$rc"

run sshpass -p x -f pw.txt true
[ $rc -eq 2 ] && ok "two password sources: exit 2 (conflicting arguments)" || bad "conflicting args: rc=$rc"

[ $fail -eq 0 ] && { echo "PASS sshpass"; exit 0; } || { echo "FAIL sshpass ($fail check(s) failed)"; exit 1; }
