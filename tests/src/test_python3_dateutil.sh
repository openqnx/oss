#!/bin/sh
# python3-dateutil: parse dates in many formats, relativedelta arithmetic
# (month ends, leap years), recurrence rules, Easter, fixed-offset zones and
# the zoneinfo database bundled with dateutil (QNX has no system zoneinfo).
echo "== python3-dateutil =="
PY=${QNX_PORT_PYTHON:-/usr/local/bin/python3}; [ -x "$PY" ] || PY=python3
T=$(mktemp -d "${TMPDIR:-/tmp}/qnxport-dateutil.XXXXXX") || { echo "FAIL python3-dateutil (no temporary directory)"; exit 1; }
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
import datetime as dt
from dateutil import parser, relativedelta, rrule, easter, tz
from dateutil.zoneinfo import get_zonefile_instance
import dateutil
info("dateutil %s" % dateutil.__version__)
def parsing():
    d = parser.parse("2024-03-10T14:30:00+05:30")
    check(d.utcoffset() == dt.timedelta(hours=5, minutes=30) and d.hour == 14, "ISO 8601 with +05:30")
    check(parser.parse("March 5, 2021 4pm") == dt.datetime(2021, 3, 5, 16, 0), "\"March 5, 2021 4pm\"")
    check(parser.parse("05/04/2021", dayfirst=True) == dt.datetime(2021, 4, 5), "dayfirst 05/04/2021 -> 5 April")
    check(parser.parse("Thu, 01 Feb 2024 10:00:00 GMT").tzinfo is not None, "RFC 2822 date with GMT")
    check(parser.isoparse("2024-W10-3") == dt.datetime(2024, 3, 6), "isoparse ISO week date 2024-W10-3")
    try: parser.parse("not a date"); check(False, "garbage rejected")
    except (ValueError, OverflowError): check(True, "garbage rejected")
def rel():
    rd = relativedelta.relativedelta
    check(dt.date(2024, 1, 31) + rd(months=+1) == dt.date(2024, 2, 29), "Jan 31 + 1 month = Feb 29 (2024)")
    check(dt.date(2023, 1, 31) + rd(months=+1) == dt.date(2023, 2, 28), "Jan 31 + 1 month = Feb 28 (2023)")
    r = rd(dt.date(2024, 7, 1), dt.date(1990, 2, 15))
    check((r.years, r.months, r.days) == (34, 4, 16), "relativedelta between dates: 34y 4m 16d")
    check(dt.date(2024, 5, 15) + rd(weekday=relativedelta.FR(+1)) == dt.date(2024, 5, 17), "next Friday")
def rules():
    days = list(rrule.rrule(rrule.WEEKLY, count=4, byweekday=(rrule.MO, rrule.TH), dtstart=dt.datetime(2024, 1, 1)))
    check([d.day for d in days] == [1, 4, 8, 11], "weekly on Mon/Thu: Jan 1, 4, 8, 11")
    last = list(rrule.rrule(rrule.MONTHLY, count=3, bymonthday=-1, dtstart=dt.datetime(2024, 1, 1)))
    check([d.day for d in last] == [31, 29, 31], "last day of month: 31, 29, 31")
    s = rrule.rrulestr("FREQ=DAILY;INTERVAL=10;COUNT=3", dtstart=dt.datetime(2024, 1, 1))
    check([d.day for d in s] == [1, 11, 21], "rrulestr FREQ=DAILY;INTERVAL=10")
    check(easter.easter(2024) == dt.date(2024, 3, 31) and easter.easter(2025) == dt.date(2025, 4, 20), "Easter 2024, 2025")
def zones():
    off = tz.tzoffset("X", -7200)
    check(dt.datetime(2024, 1, 1, tzinfo=off).utcoffset() == dt.timedelta(hours=-2), "tzoffset -02:00")
    check(tz.tzutc().utcoffset(None) == dt.timedelta(0) and tz.gettz("UTC") is not None, "UTC")
    zi = get_zonefile_instance()
    paris = zi.get("Europe/Paris")
    check(paris is not None and len(zi.zones) > 300, "bundled zoneinfo: Europe/Paris among %d zones" % len(zi.zones))
    if paris:
        check(dt.datetime(2024, 7, 1, 12, tzinfo=paris).utcoffset() == dt.timedelta(hours=2), "Paris in July: +02:00")
        check(dt.datetime(2024, 1, 1, 12, tzinfo=paris).utcoffset() == dt.timedelta(hours=1), "Paris in January: +01:00")
        check(tz.datetime_exists(dt.datetime(2024, 3, 31, 2, 30), paris) is False, "02:30 on 2024-03-31 does not exist in Paris (DST gap)")
for n, f in (("parse", parsing), ("relativedelta", rel), ("rrule", rules), ("zones", zones)):
    run(n, f)
sys.exit(1 if fails else 0)
P
"$PY" t.py; rc=$?
cd /; rm -rf "$T"
[ $rc -eq 0 ] && { echo "PASS python3-dateutil"; exit 0; }
echo "FAIL python3-dateutil (rc=$rc)"; exit 1
