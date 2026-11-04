#!/bin/sh
# gnome-keyring headless: a private D-Bus session bus, gnome-keyring-daemon
# started on it with only the secrets component and a login keyring
# unlocked from stdin, and libsecret's secret-tool as the client of the
# org.freedesktop.secrets service. Checks: the daemon owns
# org.freedesktop.secrets; secret-tool store / lookup / search round-trip
# a password with attributes; the login keyring is written to
# $XDG_DATA_HOME/keyrings and holds no plain-text password; a second
# "gnome-keyring-daemon --start" talks to the running one over its control
# socket (peer credentials, SCM_CREDS on QNX);
# secret-tool clear removes the item.
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
N=gnome-keyring
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
T=$(mktemp -d /tmp/qnxport-$N.XXXXXX) && cd "$T" || { echo "FAIL $N (no temp dir)"; exit 1; }
QP=$QNXPORT_INNER.pids; : > "$QP"
track() { echo "$1" >> "$QP"; }
stopall() { for p in $(cat "$QP" 2>/dev/null); do kill "$p" 2>/dev/null; done; : > "$QP"; sleep 1; cd /; rm -rf "$T"; }
trap stopall EXIT
to() { if command -v timeout >/dev/null 2>&1; then timeout "$@"; else shift; "$@"; fi; }
mkdir -m 700 run home data
export HOME=$T/home XDG_RUNTIME_DIR=$T/run XDG_DATA_HOME=$T/data XDG_CONFIG_HOME=$T/home/.config
cat > bus.conf <<EOS
<!DOCTYPE busconfig PUBLIC "-//freedesktop//DTD D-Bus Bus Configuration 1.0//EN"
 "http://www.freedesktop.org/standards/dbus/1.0/busconfig.dtd">
<busconfig><type>session</type><listen>unix:path=$T/bus</listen><allow_anonymous/>
<policy context="default"><allow send_destination="*" eavesdrop="true"/><allow eavesdrop="true"/><allow own="*"/></policy>
</busconfig>
EOS
"$L/bin/dbus-daemon" --config-file="$T/bus.conf" --nofork --nopidfile > bus.log 2>&1 &
track $!
i=0; while [ ! -S "$T/bus" ] && [ $i -lt 50 ]; do sleep 0.2 2>/dev/null || sleep 1; i=$((i+1)); done
[ -S "$T/bus" ] && ok "private dbus-daemon" || { bad "dbus-daemon: $(cat bus.log)"; echo "FAIL $N"; exit 1; }
export DBUS_SESSION_BUS_ADDRESS=unix:path=$T/bus
# the daemon: secrets only, the login keyring created and unlocked with a password on stdin
mkfifo pw
"$L/bin/gnome-keyring-daemon" --foreground --unlock --components=secrets < pw > gkd.out 2> gkd.log &
gkd=$!; track $gkd
printf 'qnx-login-pass' > pw
i=0; up=no
while [ $i -lt 60 ]; do
  "$L/bin/gdbus" call --session --dest org.freedesktop.DBus --object-path /org/freedesktop/DBus \
     --method org.freedesktop.DBus.NameHasOwner org.freedesktop.secrets 2>/dev/null | grep -q true && { up=yes; break; }
  kill -0 $gkd 2>/dev/null || break
  sleep 1; i=$((i+1))
done
[ $up = yes ] && ok "gnome-keyring-daemon owns org.freedesktop.secrets" || { bad "daemon not on the bus"; info "$(cat gkd.log gkd.out | tail -5)"; }
printf 's3cret-QNX' | to 30 "$L/bin/secret-tool" store --label="QNX test item" service qnxtest user alice 2>st.err && ok "secret-tool store (label, two attributes)" || bad "store: $(cat st.err)"
r=$(to 30 "$L/bin/secret-tool" lookup service qnxtest user alice 2>&1); [ "$r" = "s3cret-QNX" ] && ok "secret-tool lookup returns the password" || bad "lookup: $r"
r=$(to 30 "$L/bin/secret-tool" lookup service qnxtest user bob 2>&1); [ -z "$r" ] && ok "lookup with another attribute value finds nothing" || bad "lookup bob: $r"
r=$(to 30 "$L/bin/secret-tool" search --all service qnxtest 2>&1)
case "$r" in *"label = QNX test item"*"attribute.user = alice"*) ok "secret-tool search: label and attributes";; *) bad "search: $r";; esac
f=$(ls "$XDG_DATA_HOME"/keyrings/*.keyring 2>/dev/null | head -1)
[ -n "$f" ] && ok "keyring file written: ${f##*/}" || bad "no keyring file in \$XDG_DATA_HOME/keyrings"
[ -n "$f" ] && ! grep -q 's3cret-QNX' "$f" && ok "the keyring file holds no plain-text password" || bad "plain text in $f"
r=$(to 30 "$L/bin/gnome-keyring-daemon" --start --components=secrets 2>&1); rc=$?
# it reports the control-socket handshake on stderr ("discover_other_daemon: 1")
case "$r" in *"discover_other_daemon: 1"*) ok "--start reaches the running daemon over its control socket";; *) bad "--start (rc $rc): $r";; esac
grep -q "not SCM_CREDS" gkd.log && bad "the daemon refused a peer: $(grep -m1 SCM_CREDS gkd.log)" || ok "the daemon accepted the peer's SCM_CREDS credentials"
[ -S "$XDG_RUNTIME_DIR/keyring/control" ] && ok "control socket in \$XDG_RUNTIME_DIR/keyring" || bad "no control socket"
to 30 "$L/bin/secret-tool" clear service qnxtest user alice 2>/dev/null
r=$(to 30 "$L/bin/secret-tool" lookup service qnxtest user alice 2>&1); [ -z "$r" ] && ok "secret-tool clear removes the item" || bad "after clear: $r"
kill $gkd 2>/dev/null
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
info "daemon log: $(tail -5 gkd.log | tr '\n' '|')"
echo "FAIL $N ($fail check(s) failed)"; exit 1
