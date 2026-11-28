#!/bin/sh
# dconf (GSettings storage): on a private D-Bus session bus, with a home,
# config and runtime directory of its own. Checks: "dconf help" lists the
# commands; a write starts dconf-service by D-Bus activation
# (ca.desrt.dconf) and the value reads back, typed (int32, string, array);
# the database file ~/.config/dconf/user is a GVariant database; list, dump
# and load round-trip a directory; "dconf watch" in another process is told
# of a change (the shm flag in $XDG_RUNTIME_DIR/dconf and the D-Bus signal);
# reset removes a key; "dconf compile" builds a system database from keyfiles
# and a profile that names it serves its values as defaults; the GIO module
# makes dconf the GSettings backend: "gsettings set" on a schema compiled for
# the test lands in the dconf database.
. "$(dirname "$0")/qnxport-timeout.sh" || { echo "FAIL dconf (qnxport-timeout.sh missing)"; exit 1; }
echo "== dconf =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
P=${QNX_PORT_PREFIX:-/usr/local}
T=$(mktemp -d /tmp/qnxport-dconf.XXXXXX) || { echo "FAIL dconf (mktemp)"; exit 1; }
DBP=""; WP=""
trap '[ -n "$WP" ] && kill $WP 2>/dev/null; [ -n "$DBP" ] && kill $DBP 2>/dev/null; cd /; rm -rf "$T"' EXIT
trap 'exit 1' INT TERM HUP
cd "$T" || exit 1
D="qto 30 $P/bin/dconf"

HOME=$T/home; XDG_CONFIG_HOME=$T/home/.config; XDG_RUNTIME_DIR=$T/rt; XDG_CACHE_HOME=$T/home/.cache
export HOME XDG_CONFIG_HOME XDG_RUNTIME_DIR XDG_CACHE_HOME
unset DCONF_PROFILE GSETTINGS_BACKEND
mkdir -p "$XDG_CONFIG_HOME" "$XDG_CACHE_HOME"; mkdir -m 700 "$XDG_RUNTIME_DIR"

h=$($D help 2>&1)
echo "$h" | grep -q "watch" && echo "$h" | grep -q "compile" && echo "$h" | grep -q "dump" \
  && ok "dconf help lists watch, compile and dump" || bad "dconf help: $(echo "$h" | head -n 3)"
[ -f "$P/share/dbus-1/services/ca.desrt.dconf.service" ] && [ -x "$P/libexec/dconf-service" ] \
  && ok "dconf-service and its D-Bus activation file are installed" || bad "dconf-service or ca.desrt.dconf.service missing"

if [ -x "$P/bin/dbus-daemon" ]; then
  DBUS_SESSION_BUS_ADDRESS="unix:path=$T/bus"; export DBUS_SESSION_BUS_ADDRESS
  "$P/bin/dbus-daemon" --config-file="$P/share/dbus-1/session.conf" --address="$DBUS_SESSION_BUS_ADDRESS" --nofork --nopidfile > dbus.log 2>&1 &
  DBP=$!
  i=0; while [ $i -lt 10 ] && [ ! -S "$T/bus" ]; do sleep 1; i=$((i+1)); done
  [ -S "$T/bus" ] && ok "private D-Bus session bus" || bad "dbus-daemon: $(head -n 2 dbus.log)"
else bad "dbus-daemon not installed"; fi

# --- write (activates dconf-service), read back, typed values ---
$D write /org/qnxport/test/number 42 > w.out 2>&1 && ok "dconf write (dconf-service started by the bus)" || bad "dconf write: $(head -n 3 w.out)"
v=$($D read /org/qnxport/test/number 2>&1); [ "$v" = "42" ] && ok "read back int32 42" || bad "read number: $v"
$D write /org/qnxport/test/name "'QNX desktop'" > /dev/null 2>&1
v=$($D read /org/qnxport/test/name 2>&1); [ "$v" = "'QNX desktop'" ] && ok "read back a string" || bad "read name: $v"
$D write /org/qnxport/test/list "['a', 'b', 'c']" > /dev/null 2>&1
v=$($D read /org/qnxport/test/list 2>&1); [ "$v" = "['a', 'b', 'c']" ] && ok "read back a string array" || bad "read list: $v"
$D write /org/qnxport/test/bad "not a gvariant (" > e.out 2>&1 && bad "a malformed value was accepted" || ok "a malformed value is refused"

DB=$XDG_CONFIG_HOME/dconf/user
hd=$(head -c 8 "$DB" 2>/dev/null)
[ "$hd" = "GVariant" ] && ok "database $DB is a GVariant database ($(wc -c < "$DB" | tr -d ' ') bytes)" || bad "database file: $(ls -l "$DB" 2>&1)"
[ -f "$XDG_RUNTIME_DIR/dconf/user" ] && ok "shm flag file in \$XDG_RUNTIME_DIR/dconf" || bad "no $XDG_RUNTIME_DIR/dconf/user"

# --- list, dump, load ---
l=$($D list /org/qnxport/test/ 2>&1 | sort | tr '\n' ' ')
[ "$l" = "list name number " ] && ok "dconf list: $l" || bad "dconf list: $l"
$D dump /org/qnxport/ > dump.ini 2> dump.err
grep -q '^\[test\]' dump.ini && grep -q '^number=42$' dump.ini && grep -q "^name='QNX desktop'$" dump.ini \
  && ok "dconf dump writes the keyfile form" || bad "dconf dump: $(head -n 5 dump.ini dump.err)"
printf '[panel]\nsize=uint32 28\nautohide=true\n' | $D load /org/qnxport/loaded/ > load.out 2>&1
v=$($D read /org/qnxport/loaded/panel/size 2>&1); a=$($D read /org/qnxport/loaded/panel/autohide 2>&1)
[ "$v" = "uint32 28" ] && [ "$a" = "true" ] && ok "dconf load: uint32 and boolean keys" || bad "dconf load: size=$v autohide=$a $(head -n 2 load.out)"

# --- watch: another process sees a change ---
"$P/bin/dconf" watch /org/qnxport/test/ > watch.out 2>&1 &
WP=$!
sleep 2
$D write /org/qnxport/test/number 43 > /dev/null 2>&1
i=0; while [ $i -lt 10 ] && ! grep -q "43" watch.out 2>/dev/null; do sleep 1; i=$((i+1)); done
kill $WP 2>/dev/null; WP=""
grep -q "/org/qnxport/test/number" watch.out && grep -q "43" watch.out && ok "dconf watch reported the change" || bad "dconf watch: $(head -n 4 watch.out)"

# --- reset ---
$D reset /org/qnxport/test/name > /dev/null 2>&1
v=$($D read /org/qnxport/test/name 2>&1); [ -z "$v" ] && ok "reset removed the key" || bad "after reset: $v"
$D reset -f /org/qnxport/loaded/ > /dev/null 2>&1
v=$($D list /org/qnxport/ 2>&1 | tr '\n' ' '); [ "$v" = "test/ " ] && ok "reset -f removed a directory" || bad "after reset -f: $v"

# --- compile a system database; a profile layers it under the user's ---
mkdir -p sys.d
printf '[org/qnxport/defaults]\nwallpaper='"'"'/usr/local/share/backgrounds/default.png'"'"'\nnumber=7\n' > sys.d/00-defaults
$D compile "$T/site" "$T/sys.d" > compile.out 2>&1
[ "$(head -c 8 "$T/site" 2>/dev/null)" = "GVariant" ] && ok "dconf compile built a database from keyfiles" || bad "dconf compile: $(head -n 3 compile.out)"
printf 'user-db:user\nfile-db:%s\n' "$T/site" > profile
v=$(DCONF_PROFILE=$T/profile $D read /org/qnxport/defaults/number 2>&1)
[ "$v" = "7" ] && ok "profile: default from the compiled database" || bad "profile default: $v"
DCONF_PROFILE=$T/profile $D write /org/qnxport/defaults/number 8 > /dev/null 2>&1
v=$(DCONF_PROFILE=$T/profile $D read /org/qnxport/defaults/number 2>&1)
d=$(DCONF_PROFILE=$T/profile $D read -d /org/qnxport/defaults/number 2>&1)
[ "$v" = "8" ] && [ "$d" = "7" ] && ok "profile: the user's value overrides, read -d still gives the default" || bad "profile override: value=$v default=$d"

# --- GSettings through the GIO module ---
M=$P/lib/gio/modules/libdconfsettings.so
[ -f "$M" ] && ok "GIO module libdconfsettings.so installed" || bad "no $M"
if [ -x "$P/bin/glib-compile-schemas" ] && [ -x "$P/bin/gsettings" ]; then
  mkdir -p schemas
  cat > schemas/org.qnxport.test.gschema.xml <<'EOF'
<schemalist>
  <schema id="org.qnxport.test" path="/org/qnxport/gs/">
    <key name="greeting" type="s"><default>'hello'</default></key>
    <key name="count" type="i"><default>1</default></key>
  </schema>
</schemalist>
EOF
  qto 30 "$P/bin/glib-compile-schemas" schemas > gcs.out 2>&1 || bad "glib-compile-schemas: $(head -n 2 gcs.out)"
  GS="qto 30 env GSETTINGS_SCHEMA_DIR=$T/schemas GSETTINGS_BACKEND=dconf GIO_EXTRA_MODULES=$P/lib/gio/modules $P/bin/gsettings"
  v=$($GS get org.qnxport.test greeting 2>&1); [ "$v" = "'hello'" ] && ok "gsettings get: the schema default" || bad "gsettings get: $v"
  $GS set org.qnxport.test count 5 > gs.out 2>&1
  $GS set org.qnxport.test greeting "'annyeong'" >> gs.out 2>&1
  v=$($D read /org/qnxport/gs/count 2>&1); g=$($D read /org/qnxport/gs/greeting 2>&1)
  [ "$v" = "5" ] && [ "$g" = "'annyeong'" ] && ok "gsettings set is stored in dconf (backend loaded)" || bad "gsettings -> dconf: count=$v greeting=$g $(head -n 3 gs.out)"
  v=$($GS get org.qnxport.test count 2>&1); [ "$v" = "5" ] && ok "gsettings get reads it back from dconf" || bad "gsettings get count: $v"
else bad "glib-compile-schemas or gsettings not installed"; fi

[ $fail -eq 0 ] && { echo "PASS dconf"; exit 0; }
echo "FAIL dconf ($fail check(s) failed)"; exit 1
