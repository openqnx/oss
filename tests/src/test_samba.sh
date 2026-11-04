#!/bin/sh
# Samba as a standalone file server on 127.0.0.1 and its clients, in a
# scratch directory (every state/lock/private/log path of smb.conf points
# there; nothing system-wide is written). Checks: testparm accepts the
# configuration; smbpasswd adds a tdbsam user; smbd serves on port 4455
# (unprivileged port, no NetBIOS); smbclient -L lists the share; smbclient
# with that user gets a file (same bytes), puts one (lands in the share),
# makes and lists a directory, deletes a file, and is refused with a wrong
# password (NT_STATUS_LOGON_FAILURE); an SMB3 connection with encryption
# required (-e, --max-protocol SMB3) copies a file too; smbget fetches
# over smb:// URL; smbstatus shows the session; nmbd answers a unicast
# name query from nmblookup on 127.0.0.1. QNX has no "nobody" account, so
# the guest account is root (guests stay refused: map to guest = Never).
# smbd looks up its own host name (get_mydnsfullname) while answering the
# first negotiate; the VMs' /etc/hosts lacks it and the DHCP DNS takes 30 s
# per try to fail (hazard 144), longer than smbclient's 20 s negotiation
# timeout: RES_OPTIONS makes the resolver give up after one 1 s try, and
# the clients wait longer (-t). (Not a Samba defect: name the host in
# /etc/hosts on a real target.)
if [ -z "$QNXPORT_INNER" ]; then
  QNXPORT_INNER=/tmp/qnxport-out.$$; export QNXPORT_INNER
  sh "$0" "$@" > "$QNXPORT_INNER" 2>&1 < /dev/null &
  qp=$!; qi=0
  while kill -0 $qp 2>/dev/null && [ $qi -lt 600 ]; do sleep 1; qi=$((qi+1)); done
  if kill -0 $qp 2>/dev/null; then
    for p in $(cat "$QNXPORT_INNER.pids" 2>/dev/null); do kill $p 2>/dev/null; done
    kill $qp 2>/dev/null; sleep 1; kill -KILL $qp 2>/dev/null
    cat "$QNXPORT_INNER"; echo "  FAIL watchdog: still running after 600 s -- stopped"
    echo "FAIL ${0##*/} (watchdog)"; rm -f "$QNXPORT_INNER" "$QNXPORT_INNER.pids"; exit 1
  fi
  wait $qp; rc=$?; cat "$QNXPORT_INNER"; rm -f "$QNXPORT_INNER" "$QNXPORT_INNER.pids"; exit $rc
fi
N=samba
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
B=$L/bin; SB=$L/sbin
T=$(mktemp -d /tmp/qnxport-$N.XXXXXX) && cd "$T" || { echo "FAIL $N (no temp dir)"; exit 1; }
QP=$QNXPORT_INNER.pids; : > "$QP"
stopall() {
  for f in "$T"/st/pid/*.pid; do [ -f "$f" ] && kill "$(cat "$f")" 2>/dev/null; done
  for p in $(cat "$QP" 2>/dev/null); do kill "$p" 2>/dev/null; done; : > "$QP"
  sleep 1; cd /; rm -rf "$T"
}
trap stopall EXIT
RES_OPTIONS="timeout:1 attempts:1"; export RES_OPTIONS
to() { if command -v timeout >/dev/null 2>&1; then timeout "$@"; else shift; "$@"; fi; }
mkdir -p st/private st/lock st/state st/cache st/pid st/log st/ncalrpc share
printf 'hello from QNX samba\n' > share/hello.txt
dd if=/dev/urandom of=big.bin bs=1024 count=512 2>/dev/null
cat > smb.conf <<EOS
[global]
  netbios name = QNXTEST
  workgroup = QNXGROUP
  server role = standalone server
  security = user
  map to guest = Never
  guest account = root
  passdb backend = tdbsam
  dbwrap_tdb_mutexes:* = no
  private dir = $T/st/private
  lock directory = $T/st/lock
  state directory = $T/st/state
  cache directory = $T/st/cache
  pid directory = $T/st/pid
  ncalrpc dir = $T/st/ncalrpc
  log file = $T/st/log/log.%m
  log level = 1
  smb ports = 4455
  interfaces = 127.0.0.1
  bind interfaces only = yes
  load printers = no
  printing = bsd
  printcap name = /dev/null
  disable spoolss = yes
[pub]
  path = $T/share
  read only = no
  comment = QNX test share
EOS
C="-s $T/smb.conf"
r=$("$B/testparm" $C -s 2>&1); case "$r" in *"[pub]"*"path = $T/share"*) ok "testparm accepts smb.conf";; *) bad "testparm: $r";; esac
r=$(printf 'Qnx-Pass1\nQnx-Pass1\n' | to 60 "$B/smbpasswd" -c "$T/smb.conf" -a -s root 2>&1); case "$r" in *"Added user root"*) ok "smbpasswd -a: tdbsam user root";; *) bad "smbpasswd: $r";; esac
"$SB/smbd" $C -D 2> smbd.err
i=0; up=no
while [ $i -lt 3 ]; do
  to 100 "$B/smbclient" $C -t 90 -p 4455 -U root%Qnx-Pass1 //127.0.0.1/pub -c 'ls' > ls.out 2>&1 && { up=yes; break; }
  sleep 1; i=$((i+1))
done
[ $up = yes ] && ok "smbd answers on 127.0.0.1:4455" || { bad "smbd not answering: $(cat ls.out smbd.err | tail -3)"; info "$(tail -15 st/log/log.smbd 2>/dev/null | tr '\n' '|')"; echo "FAIL $N"; exit 1; }
grep -q "hello.txt" ls.out && ok "smbclient ls: hello.txt" || bad "ls: $(cat ls.out)"
r=$(to 120 "$B/smbclient" $C -t 90 -p 4455 -U root%Qnx-Pass1 -L 127.0.0.1 2>&1); case "$r" in *pub*"QNX test share"*) ok "smbclient -L lists [pub] with its comment";; *) bad "-L: $r";; esac
sc() { to 120 "$B/smbclient" $C -t 90 -p 4455 -U root%Qnx-Pass1 //127.0.0.1/pub "$@" 2>&1; }
sc -c "get hello.txt $T/got.txt" >/dev/null; cmp -s got.txt share/hello.txt && ok "get: the same bytes" || bad "get"
sc -c "put $T/big.bin up.bin" >/dev/null; cmp -s big.bin share/up.bin && ok "put 512 KiB: lands in the share intact" || bad "put"
r=$(sc -c "mkdir newdir; ls"); [ -d share/newdir ] && case "$r" in *newdir*) true;; *) false;; esac && ok "mkdir, listed" || bad "mkdir: $r"
r=$(sc -c "del up.bin"); [ ! -e share/up.bin ] && ok "del removes the file" || bad "del: $r"
r=$(to 120 "$B/smbclient" $C -t 90 -p 4455 -U root%wrong //127.0.0.1/pub -c ls 2>&1); case "$r" in *LOGON_FAILURE*) ok "wrong password: NT_STATUS_LOGON_FAILURE";; *) bad "wrong password: $r";; esac
r=$(sc --client-protection=encrypt --max-protocol=SMB3 -c "get hello.txt $T/got3.txt" ); cmp -s got3.txt share/hello.txt && ok "SMB3 with encryption required (--client-protection=encrypt): get" || bad "SMB3 encrypt: $r"
rm -f hello.txt; r=$(cd "$T" && to 120 "$B/smbget" $C -U root%Qnx-Pass1 smb://127.0.0.1:4455/pub/hello.txt 2>&1); cmp -s "$T/hello.txt" share/hello.txt && ok "smbget smb://127.0.0.1:4455/pub/hello.txt" || bad "smbget: $r"
r=$(to 30 "$B/smbstatus" $C -b 2>&1); case "$r" in *"Samba version 4.25.1"*) ok "smbstatus -b";; *) bad "smbstatus: $r";; esac
# nmbd: NetBIOS name service. It needs a non-loopback IPv4 interface (on
# loopback only it waits for one), so it serves the target's first one,
# UDP 137 (root on the target); nmblookup asks it by unicast.
IP=$(ifconfig 2>/dev/null | awk '/inet / && $2 != "127.0.0.1" {print $2; exit}')
if [ -n "$IP" ]; then
  sed "s|^  interfaces = 127.0.0.1|  interfaces = 127.0.0.1 $IP/32|" smb.conf > smb2.conf && mv smb2.conf smb.conf
  "$SB/nmbd" $C -D 2> nmbd.err
  i=0; r=""
  while [ $i -lt 30 ]; do r=$(to 10 "$B/nmblookup" $C -U "$IP" QNXTEST 2>&1); case "$r" in *"QNXTEST<00>"*) break;; esac; sleep 1; i=$((i+1)); done
  case "$r" in *"$IP QNXTEST<00>"*) ok "nmblookup -U $IP QNXTEST: nmbd answers with $IP";; *) bad "nmblookup: $r"; info "$(tail -8 st/log/log.nmbd 2>/dev/null | tr '\n' '|')";; esac
else info "no non-loopback IPv4 interface: nmbd not checked"; fi
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
info "smbd log: $(tail -12 st/log/log.smbd 2>/dev/null | tr '\n' '|')"
echo "FAIL $N ($fail check(s) failed)"; exit 1
