#!/bin/sh
# python3-tzdata: the IANA database as a Python package -- what the standard
# zoneinfo module falls back to on QNX, which has no system zoneinfo. With
# zoneinfo's search path emptied, zones must come from tzdata: binary TZif
# data, known offsets and DST rules, abbreviations, and the zone list.
echo "== python3-tzdata =="
PY=${QNX_PORT_PYTHON:-/usr/local/bin/python3}; [ -x "$PY" ] || PY=python3
T=$(mktemp -d "${TMPDIR:-/tmp}/qnxport-tzdata.XXXXXX") || { echo "FAIL python3-tzdata (no temporary directory)"; exit 1; }
cd "$T" || exit 1
cat > t.py <<'P'
import sys, os
fails = 0
def check(ok, what):
    global fails
    print(("  ok   " if ok else "  FAIL ") + what); fails += (not ok)
def info(s): print("  info " + s)
def run(name, fn):
    try: fn()
    except Exception as e:
        import traceback; traceback.print_exc(limit=3, file=sys.stdout)
        check(False, "%s: %r" % (name, e))
import datetime as dt, importlib.resources, zoneinfo, tzdata
info("tzdata %s (IANA %s)" % (tzdata.__version__, tzdata.IANA_VERSION))
zoneinfo.reset_tzpath(to=[])                    # only the tzdata package may answer
def data():
    raw = importlib.resources.files("tzdata.zoneinfo").joinpath("Europe/Berlin").read_bytes()
    check(raw[:4] == b"TZif" and raw[4:5] in (b"2", b"3", b"4"), "zoneinfo/Europe/Berlin is TZif data (v%s)" % raw[4:5].decode())
    check(tzdata.IANA_VERSION == "2025b", "IANA release 2025b")
    names = importlib.resources.files("tzdata").joinpath("zones").read_text().split()
    check(len(names) > 500 and "America/Sao_Paulo" in names, "%d zones listed" % len(names))
def zones():
    zoneinfo.ZoneInfo.clear_cache()
    b = zoneinfo.ZoneInfo("Europe/Berlin")
    check(dt.datetime(2024, 1, 1, 12, tzinfo=b).utcoffset() == dt.timedelta(hours=1), "Berlin in January: +01:00 (CET)")
    check(dt.datetime(2024, 7, 1, 12, tzinfo=b).tzname() == "CEST", "Berlin in July: CEST")
    t = dt.datetime(2024, 3, 31, 0, 30, tzinfo=dt.timezone.utc).astimezone(b)
    u = dt.datetime(2024, 3, 31, 1, 30, tzinfo=dt.timezone.utc).astimezone(b)
    check((t.hour, u.hour) == (1, 3), "DST starts 2024-03-31 01:00 UTC (01:30 -> 03:30 local)")
    check(dt.datetime(2024, 6, 1, tzinfo=zoneinfo.ZoneInfo("Asia/Tokyo")).utcoffset() == dt.timedelta(hours=9), "Tokyo +09:00, no DST")
    check(dt.datetime(2024, 1, 1, tzinfo=zoneinfo.ZoneInfo("Asia/Kathmandu")).utcoffset() == dt.timedelta(hours=5, minutes=45), "Kathmandu +05:45")
    check(dt.datetime(1970, 1, 1, tzinfo=zoneinfo.ZoneInfo("Europe/London")).utcoffset() == dt.timedelta(hours=1), "London 1970: +01:00 (British Standard Time)")
    check(dt.datetime(2024, 11, 3, 1, 30, fold=1, tzinfo=zoneinfo.ZoneInfo("America/New_York")).tzname() == "EST", "New York fold=1 at 01:30 on 2024-11-03 is EST")
    try: zoneinfo.ZoneInfo("Nowhere/Atlantis"); check(False, "unknown zone rejected")
    except zoneinfo.ZoneInfoNotFoundError: check(True, "unknown zone rejected")
    check(len(zoneinfo.available_timezones()) > 500, "available_timezones() from tzdata")
for n, f in (("data", data), ("zoneinfo", zones)):
    run(n, f)
sys.exit(1 if fails else 0)
P
"$PY" t.py; rc=$?
cd /; rm -rf "$T"
[ $rc -eq 0 ] && { echo "PASS python3-tzdata"; exit 0; }
echo "FAIL python3-tzdata (rc=$rc)"; exit 1
