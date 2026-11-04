#!/bin/sh
# tzdata: the target's zdump and zic, and Python's zoneinfo on the zoneinfo
# files. Known answers: Berlin's 2024 DST transitions as zdump reports them;
# a zone compiled on the target by zic from a source line and read back by
# zdump; Python's zoneinfo (TZPATH pointed at share/zoneinfo) converting
# across a transition and listing the zones.
echo "== tzdata-tools =="
P=${QNX_PORT_PREFIX:-/usr/local}
Z=$P/share/zoneinfo
fails=0
ok()   { echo "  ok   $1"; }
bad()  { echo "  FAIL $1"; fails=$((fails + 1)); }
chk()  { if [ "$2" = "$3" ]; then ok "$1"; else bad "$1"; echo "  info want: $3"; echo "  info got:  $2"; fi; }

[ -x "$P/bin/zdump" ] && ok "zdump installed" || bad "zdump installed ($P/bin/zdump)"
out=$("$P/bin/zdump" -V -c 2024,2025 "$Z/Europe/Berlin" 2>&1 | sed "s|$Z/||")
want="Europe/Berlin  Sun Mar 31 00:59:59 2024 UT = Sun Mar 31 01:59:59 2024 CET isdst=0 gmtoff=3600
Europe/Berlin  Sun Mar 31 01:00:00 2024 UT = Sun Mar 31 03:00:00 2024 CEST isdst=1 gmtoff=7200
Europe/Berlin  Sun Oct 27 00:59:59 2024 UT = Sun Oct 27 02:59:59 2024 CEST isdst=1 gmtoff=7200
Europe/Berlin  Sun Oct 27 01:00:00 2024 UT = Sun Oct 27 02:00:00 2024 CET isdst=0 gmtoff=3600"
chk "zdump -V -c 2024,2025 Europe/Berlin: the two 2024 transitions" "$out" "$want"

# zic on the target: a made-up zone, +03:30 with DST from 2030 on
T=${TMPDIR:-/tmp}/tzdata-test.$$; mkdir -p "$T"
cat > "$T/test.zi" <<'ZI'
Rule	Tst	2030	max	-	Apr	Sun>=1	2:00	1:00	D
Rule	Tst	2030	max	-	Oct	Sun>=1	2:00	0	S
Zone	Test/Zone	3:30	Tst	+0330/+0430
ZI
if "$P/bin/zic" -d "$T/zi" "$T/test.zi" 2>"$T/err"; then ok "zic compiles a zone source"; else bad "zic compiles a zone source"; cat "$T/err"; fi
out=$("$P/bin/zdump" -V -c 2030,2031 "$T/zi/Test/Zone" 2>&1 | sed 's/.* = //')
# first Sunday of April 2030 is the 7th, of October the 6th; 02:00 local
want="Sun Apr  7 01:59:59 2030 +0330 isdst=0 gmtoff=12600
Sun Apr  7 03:00:00 2030 +0430 isdst=1 gmtoff=16200
Sun Oct  6 01:59:59 2030 +0430 isdst=1 gmtoff=16200
Sun Oct  6 01:00:00 2030 +0330 isdst=0 gmtoff=12600"
chk "zdump of the zic-compiled zone: 2030 spring and autumn transitions" "$out" "$want"
tail -n 1 "$T/zi/Test/Zone" | grep -q '^<+0330>-3:30<+0430>,M4.1.0,M10.1.0$' && ok "its POSIX footer <+0330>-3:30<+0430>,M4.1.0,M10.1.0" \
  || { bad "its POSIX footer"; tail -n 1 "$T/zi/Test/Zone"; }
rm -rf "$T"

PY=$P/bin/python3
if [ -x "$PY" ]; then
  out=$(PYTHONTZPATH=$Z "$PY" - "$Z" 2>&1 <<'PYEOF'
import sys, zoneinfo, datetime
z = sys.argv[1]
print("tzpath", zoneinfo.TZPATH[0] == z)
b = zoneinfo.ZoneInfo("Europe/Berlin")
u = datetime.timezone.utc
print(datetime.datetime(2024, 3, 31, 0, 30, tzinfo=u).astimezone(b).isoformat(), b.tzname(datetime.datetime(2024, 3, 31, 0, 30)))
print(datetime.datetime(2024, 3, 31, 1, 30, tzinfo=u).astimezone(b).isoformat(), b.tzname(datetime.datetime(2024, 3, 31, 4, 0)))
with open(z + "/America/Sao_Paulo", "rb") as f:
    sp = zoneinfo.ZoneInfo.from_file(f)
print(datetime.datetime(2018, 12, 1, tzinfo=u).astimezone(sp).isoformat())
a = zoneinfo.available_timezones()
print("zones", len(a) > 400, "Asia/Kolkata" in a, "Europe/Kyiv" in a)
PYEOF
)
  want="tzpath True
2024-03-31T01:30:00+01:00 CET
2024-03-31T03:30:00+02:00 CEST
2018-11-30T22:00:00-02:00
zones True True True"
  chk "python3 zoneinfo on share/zoneinfo: Berlin across the 2024 switch, Sao Paulo 2018 DST, the zone list" "$out" "$want"
else
  echo "  info python3 not installed: zoneinfo check skipped"
fi
[ $fails -eq 0 ] && { echo "PASS tzdata-tools"; exit 0; }
echo "FAIL tzdata-tools ($fails)"; exit 1
