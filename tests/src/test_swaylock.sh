#!/bin/sh
# swaylock (screen locker): against a headless labwc (pixman renderer).
# Checks: --version names 1.8.6; --help lists the options; the PAM policy
# etc/pam.d/swaylock is installed and the "login" policy it includes exists
# on the target; "swaylock --ready-fd" locks
# the session (ext-session-lock-v1: the compositor answers "locked", swaylock
# writes to the ready descriptor) and keeps running; while it is locked a
# second swaylock is turned away by the compositor; SIGUSR1 unlocks and ends
# it with status 0; a malformed colour (-c not-a-colour) is reported but
# still locks and stays locked; after that the session can be locked again, with a PNG
# background made by the test; without a compositor swaylock fails instead of
# hanging. The headless compositor has no keyboard, so typing a password is
# not part of this test: whether the user's password unlocks is a check for a
# session with input (see the port's notes on pam_qnx).
. "$(dirname "$0")/qnxport-timeout.sh" || { echo "FAIL swaylock (qnxport-timeout.sh missing)"; exit 1; }
echo "== swaylock =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
P=${QNX_PORT_PREFIX:-/usr/local}
T=$(mktemp -d /tmp/qnxport-swaylock.XXXXXX) || { echo "FAIL swaylock (mktemp)"; exit 1; }
LWP=""; SP=""
trap '[ -n "$SP" ] && kill -USR1 $SP 2>/dev/null; [ -n "$LWP" ] && kill $LWP 2>/dev/null; cd /; rm -rf "$T"' EXIT
trap 'exit 1' INT TERM HUP
cd "$T" || exit 1
S=$P/bin/swaylock
HOME=$T/home; XDG_CONFIG_HOME=$HOME/.config; export HOME XDG_CONFIG_HOME; mkdir -p "$XDG_CONFIG_HOME"

v=$(qto 30 "$S" --version 2>&1 | head -n 1)
case "$v" in "swaylock version"*"1.8.6"*) ok "$v" ;; *) bad "swaylock --version: $v" ;; esac
h=$(qto 30 "$S" --help 2>&1)
echo "$h" | grep -q -- "--daemonize" && echo "$h" | grep -q -- "--image" && echo "$h" | grep -q -- "--ready-fd" \
  && ok "swaylock --help lists --daemonize, --image and --ready-fd" || bad "swaylock --help: $(echo "$h" | head -n 3)"
if grep -q "^auth" "$P/etc/pam.d/swaylock" 2>/dev/null; then
  ok "PAM policy $P/etc/pam.d/swaylock: $(grep -m1 '^auth' "$P/etc/pam.d/swaylock" | tr -s ' \t' ' ')"
  # The policy names QNX's module itself, by its path in the image (a mkqnximage
  # target has no /etc/pam.d/login to include, and /system/lib/dll/pam is not
  # on OpenPAM's module path). OpenPAM loads it from that path if root owns it
  # and nobody else may write it.
  m=$(awk '$1 == "auth" {print $3; exit}' "$P/etc/pam.d/swaylock")
  case "$m" in
    /*pam_qnx.so)
      if [ -f "$m" ]; then
        own=$(ls -ln "$m" | awk '{print $3}'); perm=$(ls -l "$m" | cut -c6,9)
        [ "$own" = 0 ] && [ "$perm" = "--" ] && ok "the policy's module $m is on the target, root's, not group/world-writable" \
          || bad "$m: owner $own, group/other write '$perm' (OpenPAM refuses it)"
      else bad "the policy's module $m is not on the target (another BSP image: change the path)"; fi ;;
    *) bad "the policy does not name pam_qnx.so by an absolute path: '$m'" ;;
  esac
  n=$(grep -c -E '^(auth|account|session|password)[[:space:]]' "$P/etc/pam.d/swaylock")
  [ "$n" -eq 4 ] && ok "the policy covers auth, account, session and password" || bad "the policy has $n of the 4 facilities (OpenPAM falls back to a missing 'other')"
else bad "no PAM policy $P/etc/pam.d/swaylock"; fi

# --- no compositor: an error, not a hang ---
mkdir -m 700 "$T/empty"
XDG_RUNTIME_DIR=$T/empty WAYLAND_DISPLAY=wayland-9 qto 20 "$S" -c 000000 > nocomp.log 2>&1; rc=$?
[ $rc -ne 0 ] && [ $rc -ne 124 ] && ok "without a compositor: exit $rc" || bad "without a compositor: rc=$rc $(head -n 2 nocomp.log)"

RT=$T/rt; mkdir -m 700 "$RT"
XDG_RUNTIME_DIR=$RT WLR_BACKENDS=headless WLR_RENDERER=pixman "$P/bin/labwc" > labwc.log 2>&1 &
LWP=$!
i=0; while [ ! -S "$RT/wayland-0" ] && [ $i -lt 150 ]; do sleep 0.1; i=$((i+1)); done
[ -S "$RT/wayland-0" ] && ok "labwc up (headless)" || bad "labwc: $(tail -n 2 labwc.log)"
XDG_RUNTIME_DIR=$RT; WAYLAND_DISPLAY=wayland-0; export XDG_RUNTIME_DIR WAYLAND_DISPLAY

# A malformed colour is not an error for swaylock: it logs "Invalid color"
# and locks with its default. The session must stay locked then -- a typo in
# a user's colour option must not leave the screen open (checked below).

# --- lock ---
lock() {   # $1 = log name, rest = options; sets SP; the ready file gets a newline when locked
  : > "ready.$1"
  n=$1; shift
  "$S" -d --ready-fd 3 "$@" 3> "ready.$n" > "lock.$n.log" 2>&1 &
  SP=$!
  i=0; while [ $i -lt 100 ] && [ ! -s "ready.$n" ] && kill -0 $SP 2>/dev/null; do sleep 0.1; i=$((i+1)); done
  [ -s "ready.$n" ]
}
if lock a -c 102030; then ok "session locked (compositor sent ext_session_lock_v1.locked)"
else bad "lock: not locked after 10 s; $(tail -n 4 lock.a.log)"; fi
sleep 1
kill -0 $SP 2>/dev/null && ok "swaylock keeps running while locked" || bad "swaylock exited while locked: $(tail -n 4 lock.a.log)"
kill -0 $LWP 2>/dev/null && ok "labwc running with the lock surfaces" || bad "labwc died: $(tail -n 3 labwc.log)"

# --- a second locker is refused while locked ---
qto 15 "$S" -c 000000 > second.log 2>&1; rc=$?
[ $rc -ne 0 ] && [ $rc -ne 124 ] && ok "a second swaylock is turned away while locked (exit $rc)" || bad "second locker: rc=$rc $(tail -n 2 second.log)"
kill -0 $SP 2>/dev/null || bad "the first locker died when the second started: $(tail -n 3 lock.a.log)"

# --- unlock ---
kill -USR1 $SP 2>/dev/null
i=0; while [ $i -lt 50 ] && kill -0 $SP 2>/dev/null; do sleep 0.1; i=$((i+1)); done
if kill -0 $SP 2>/dev/null; then bad "SIGUSR1 did not unlock: $(tail -n 3 lock.a.log)"
else wait $SP; rc=$?; SP=""; [ $rc -eq 0 ] && ok "SIGUSR1 unlocked; swaylock ended with status 0" || bad "status after SIGUSR1: $rc $(tail -n 3 lock.a.log)"; fi

# --- a malformed colour still locks (default colour) and stays locked ---
if lock c -c not-a-colour; then ok "an invalid colour still locks the session"
else bad "invalid colour: not locked after 10 s; $(tail -n 4 lock.c.log)"; fi
grep -qi "invalid color" lock.c.log && ok "the invalid colour is reported: $(grep -i -m1 'invalid color' lock.c.log | sed 's/^.*\] //')" \
  || bad "no \"Invalid color\" message: $(tail -n 3 lock.c.log)"
sleep 1
kill -0 $SP 2>/dev/null && ok "still locked with the invalid colour" || bad "swaylock exited with the invalid colour: $(tail -n 4 lock.c.log)"
kill -USR1 $SP 2>/dev/null
i=0; while [ $i -lt 50 ] && kill -0 $SP 2>/dev/null; do sleep 0.1; i=$((i+1)); done
kill -0 $SP 2>/dev/null && bad "unlock after the invalid colour failed" || { wait $SP 2>/dev/null; SP=""; ok "unlocked after the invalid-colour lock"; }

# --- lock again, with a background image (a 2x2 PNG) ---
printf '\211\120\116\107\015\012\032\012\000\000\000\015\111\110\104\122\000\000\000\002\000\000\000\002\010\002\000\000\000\375\324\232\163\000\000\000\020\111\104\101\124\170\234\143\020\120\060\000\042\006\010\005\000\012\016\001\201\126\063\356\363\000\000\000\000\111\105\116\104\256\102\140\202' > bg.png
if lock b -i "$T/bg.png" -s fill -u; then ok "locked again, with an image background"
else bad "second lock (image): $(tail -n 4 lock.b.log)"; fi
grep -qi "failed to load\|unable to load\|error.*image" lock.b.log && bad "background image: $(grep -i -m1 'load\|image' lock.b.log)"
kill -USR1 $SP 2>/dev/null
i=0; while [ $i -lt 50 ] && kill -0 $SP 2>/dev/null; do sleep 0.1; i=$((i+1)); done
kill -0 $SP 2>/dev/null && bad "second unlock failed" || { SP=""; ok "unlocked again"; }

[ $fail -eq 0 ] && { echo "PASS swaylock"; exit 0; }
echo "FAIL swaylock ($fail check(s) failed)"; exit 1
