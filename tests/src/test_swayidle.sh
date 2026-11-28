#!/bin/sh
# swayidle (idle management daemon): against a headless labwc (pixman
# renderer), which has no input devices -- the seat is idle from the start, so
# the compositor's ext-idle-notify-v1 timers run undisturbed. Checks: -h
# prints the usage; with "timeout 1" and "timeout 3" the two commands run, in
# that order, about one and three seconds after start; SIGTERM runs the
# pending resume command and ends swayidle with status 0; a configuration
# file (-C) is read, and SIGUSR1 fires its one-hour timeout at once; without
# a compositor swayidle fails with an error instead of hanging.
. "$(dirname "$0")/qnxport-timeout.sh" || { echo "FAIL swayidle (qnxport-timeout.sh missing)"; exit 1; }
echo "== swayidle =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
P=${QNX_PORT_PREFIX:-/usr/local}
T=$(mktemp -d /tmp/qnxport-swayidle.XXXXXX) || { echo "FAIL swayidle (mktemp)"; exit 1; }
LWP=""; SP=""
trap '[ -n "$SP" ] && kill $SP 2>/dev/null; [ -n "$LWP" ] && kill $LWP 2>/dev/null; cd /; rm -rf "$T"' EXIT
trap 'exit 1' INT TERM HUP
cd "$T" || exit 1
S=$P/bin/swayidle

h=$(qto 30 "$S" -h 2>&1)
echo "$h" | grep -q "Usage:" && echo "$h" | grep -q "path to config file" && ok "swayidle -h prints the usage" || bad "swayidle -h: $(echo "$h" | head -n 3)"

# --- no compositor: an error, not a hang ---
mkdir -m 700 "$T/empty"
XDG_RUNTIME_DIR=$T/empty WAYLAND_DISPLAY=wayland-9 qto 20 "$S" timeout 1 true > nocomp.log 2>&1; rc=$?
[ $rc -ne 0 ] && [ $rc -ne 124 ] && ok "without a compositor: exit $rc ($(head -n 1 nocomp.log))" || bad "without a compositor: rc=$rc"

RT=$T/rt; mkdir -m 700 "$RT"
XDG_RUNTIME_DIR=$RT WLR_BACKENDS=headless WLR_RENDERER=pixman "$P/bin/labwc" > labwc.log 2>&1 &
LWP=$!
i=0; while [ ! -S "$RT/wayland-0" ] && [ $i -lt 150 ]; do sleep 0.1; i=$((i+1)); done
[ -S "$RT/wayland-0" ] && ok "labwc up (headless)" || bad "labwc: $(tail -n 2 labwc.log)"
XDG_RUNTIME_DIR=$RT; WAYLAND_DISPLAY=wayland-0; export XDG_RUNTIME_DIR WAYLAND_DISPLAY

# --- two timeouts, a resume command ---
"$S" -w timeout 1 "echo idle1 >> $T/ev.log" resume "echo resume1 >> $T/ev.log" timeout 3 "echo idle3 >> $T/ev.log" > idle.log 2>&1 &
SP=$!
sleep 2
if kill -0 $SP 2>/dev/null; then ok "swayidle running (ext-idle-notify-v1 bound)"; else bad "swayidle exited: $(tail -n 3 idle.log)"; fi
[ "$(cat ev.log 2>/dev/null)" = "idle1" ] && ok "after 2 s: the 1 s timeout ran, the 3 s one not yet" || bad "after 2 s: '$(tr '\n' ' ' < ev.log 2>/dev/null)' $(tail -n 2 idle.log)"
i=0; while [ $i -lt 10 ] && ! grep -q idle3 ev.log 2>/dev/null; do sleep 1; i=$((i+1)); done
[ "$(tr '\n' ' ' < ev.log 2>/dev/null)" = "idle1 idle3 " ] && ok "the 3 s timeout ran after it" || bad "timeouts: '$(tr '\n' ' ' < ev.log 2>/dev/null)'"
kill -TERM $SP 2>/dev/null
wait $SP; rc=$?; SP=""
grep -q resume1 ev.log 2>/dev/null && ok "SIGTERM ran the pending resume command" || bad "no resume on SIGTERM: '$(tr '\n' ' ' < ev.log 2>/dev/null)'"
[ $rc -eq 0 ] && ok "swayidle ended with status 0" || bad "exit status after SIGTERM: $rc"

# --- configuration file; SIGUSR1 fires the timeouts at once ---
printf '# test configuration\ntimeout 3600 "echo hour >> %s/ev2.log"\n' "$T" > cfg
"$S" -w -C "$T/cfg" > idle2.log 2>&1 &
SP=$!
sleep 2
[ ! -f ev2.log ] && kill -0 $SP 2>/dev/null && ok "configuration file read; the one-hour timeout is waiting" || bad "config run: $(tail -n 3 idle2.log) $(cat ev2.log 2>/dev/null)"
kill -USR1 $SP 2>/dev/null
i=0; while [ $i -lt 10 ] && [ ! -f ev2.log ]; do sleep 1; i=$((i+1)); done
[ "$(cat ev2.log 2>/dev/null)" = "hour" ] && ok "SIGUSR1 ran the timeout command at once" || bad "SIGUSR1: $(cat ev2.log 2>&1) $(tail -n 2 idle2.log)"
kill -TERM $SP 2>/dev/null; wait $SP 2>/dev/null; SP=""
kill -0 $LWP 2>/dev/null || bad "labwc died: $(tail -n 3 labwc.log)"

[ $fail -eq 0 ] && { echo "PASS swayidle"; exit 0; }
echo "FAIL swayidle ($fail check(s) failed)"; exit 1
