#!/bin/sh
# python3-pytz: the Olson database bundled with pytz -- localize and
# normalize across DST transitions, conversions between zones with known
# offsets (New York, London, Kolkata, Lord Howe's half hour), ambiguous and
# non-existent local times, and the zone lists.
echo "== python3-pytz =="
PY=${QNX_PORT_PYTHON:-/usr/local/bin/python3}; [ -x "$PY" ] || PY=python3
T=$(mktemp -d "${TMPDIR:-/tmp}/qnxport-pytz.XXXXXX") || { echo "FAIL python3-pytz (no temporary directory)"; exit 1; }
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
import datetime as dt, pytz
info("pytz %s (Olson %s)" % (pytz.__version__, pytz.OLSON_VERSION))
def zones():
    ny = pytz.timezone("America/New_York")
    s = ny.localize(dt.datetime(2024, 7, 1, 12, 0)); w = ny.localize(dt.datetime(2024, 1, 15, 12, 0))
    check(s.utcoffset() == dt.timedelta(hours=-4) and s.tzname() == "EDT", "New York in July: EDT -04:00")
    check(w.utcoffset() == dt.timedelta(hours=-5) and w.tzname() == "EST", "New York in January: EST -05:00")
    check(s.astimezone(pytz.utc).hour == 16, "12:00 EDT = 16:00 UTC")
    lon = pytz.timezone("Europe/London")
    check(s.astimezone(lon).strftime("%H:%M %Z") == "17:00 BST", "-> London 17:00 BST")
    kol = pytz.timezone("Asia/Kolkata")
    check(pytz.utc.localize(dt.datetime(2024, 1, 1, 0, 0)).astimezone(kol).strftime("%H:%M") == "05:30", "UTC midnight = 05:30 in Kolkata")
    lh = pytz.timezone("Australia/Lord_Howe")
    check(lh.localize(dt.datetime(2024, 1, 1)).utcoffset() == dt.timedelta(hours=11) and lh.localize(dt.datetime(2024, 7, 1)).utcoffset() == dt.timedelta(hours=10, minutes=30),
          "Lord Howe: +11:00 summer, +10:30 winter (half-hour DST)")
def dst():
    ny = pytz.timezone("America/New_York")
    before = ny.localize(dt.datetime(2024, 3, 10, 1, 30))
    after = ny.normalize(before + dt.timedelta(hours=1))
    check(after.hour == 3 and after.tzname() == "EDT", "normalize across spring forward: 01:30 EST + 1h = 03:30 EDT")
    try: ny.localize(dt.datetime(2024, 3, 10, 2, 30), is_dst=None); check(False, "02:30 on 2024-03-10 does not exist")
    except pytz.exceptions.NonExistentTimeError: check(True, "02:30 on 2024-03-10 does not exist (NonExistentTimeError)")
    try: ny.localize(dt.datetime(2024, 11, 3, 1, 30), is_dst=None); check(False, "01:30 on 2024-11-03 is ambiguous")
    except pytz.exceptions.AmbiguousTimeError: check(True, "01:30 on 2024-11-03 is ambiguous (AmbiguousTimeError)")
    a = ny.localize(dt.datetime(2024, 11, 3, 1, 30), is_dst=True); b = ny.localize(dt.datetime(2024, 11, 3, 1, 30), is_dst=False)
    check(b - a == dt.timedelta(hours=1), "the two 01:30s are an hour apart")
def lists():
    check(len(pytz.all_timezones) > 500 and len(pytz.common_timezones) > 400, "%d zones, %d common" % (len(pytz.all_timezones), len(pytz.common_timezones)))
    check("Europe/Berlin" in pytz.country_timezones["de"] and pytz.country_names["jp"] == "Japan", "country_timezones, country_names")
    try: pytz.timezone("Nowhere/Atlantis"); check(False, "unknown zone rejected")
    except pytz.exceptions.UnknownTimeZoneError: check(True, "unknown zone rejected")
    check(pytz.FixedOffset(330).utcoffset(None) == dt.timedelta(minutes=330), "FixedOffset(+330 min)")
for n, f in (("zones", zones), ("dst", dst), ("lists", lists)):
    run(n, f)
sys.exit(1 if fails else 0)
P
"$PY" t.py; rc=$?
cd /; rm -rf "$T"
[ $rc -eq 0 ] && { echo "PASS python3-pytz"; exit 0; }
echo "FAIL python3-pytz (rc=$rc)"; exit 1
