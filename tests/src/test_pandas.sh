#!/bin/sh
# pandas on the ported CPython (with numpy, dateutil, pytz, tzdata): build
# frames, group and aggregate, merge, pivot, time series (date ranges,
# resampling, time zones), string methods, missing data, and a CSV round
# trip -- each with a known result.
echo "== pandas =="
PY=${QNX_PORT_PYTHON:-/usr/local/bin/python3}; [ -x "$PY" ] || PY=python3
T=$(mktemp -d "${TMPDIR:-/tmp}/qnxport-pandas.XXXXXX") || { echo "FAIL pandas (no temporary directory)"; exit 1; }
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
import pandas as pd, numpy as np, io as _io
info("pandas %s, numpy %s" % (pd.__version__, np.__version__))
df = pd.DataFrame({"city": ["Ottawa", "Paris", "Ottawa", "Tokyo", "Paris", "Ottawa"],
                   "sales": [10, 20, 30, 40, 50, 60], "units": [1, 2, 3, 4, 5, 6]})
def groupby():
    g = df.groupby("city")["sales"].sum()
    check(g.to_dict() == {"Ottawa": 100, "Paris": 70, "Tokyo": 40}, "groupby sum: Ottawa 100, Paris 70, Tokyo 40")
    a = df.groupby("city").agg(n=("sales", "size"), mean=("sales", "mean"))
    check(a.loc["Ottawa", "n"] == 3 and a.loc["Paris", "mean"] == 35.0, "named aggregation: size, mean")
    check(df.sort_values("sales", ascending=False).iloc[0]["city"] == "Ottawa", "sort_values")
    check(df["units"].cumsum().iloc[-1] == 21 and df.describe().loc["max", "sales"] == 60, "cumsum, describe")
def merge():
    pop = pd.DataFrame({"city": ["Ottawa", "Paris", "Berlin"], "country": ["CA", "FR", "DE"]})
    m = df.merge(pop, on="city", how="left")
    check(len(m) == 6 and m["country"].isna().sum() == 1, "left merge: Tokyo has no country")
    inner = df.merge(pop, on="city")
    check(sorted(inner["country"].unique()) == ["CA", "FR"], "inner merge")
    p = df.pivot_table(index="city", values="units", aggfunc="max")
    check(p.loc["Ottawa", "units"] == 6, "pivot_table max")
    c = pd.concat([df.head(2), df.tail(2)], ignore_index=True)
    check(c.shape == (4, 3) and c.index.tolist() == [0, 1, 2, 3], "concat")
def timeseries():
    r = pd.date_range("2024-02-27", periods=4, freq="D")
    check([d.day for d in r] == [27, 28, 29, 1], "date_range across Feb 29 2024")
    s = pd.Series(range(48), index=pd.date_range("2024-01-01", periods=48, freq="h"))
    d = s.resample("D").sum()
    check(d.tolist() == [276, 852], "hourly series resampled to daily sums")
    t = pd.Timestamp("2024-07-01 12:00", tz="UTC").tz_convert("America/New_York")
    check(t.hour == 8 and t.utcoffset().total_seconds() == -4 * 3600, "UTC 12:00 -> New York 08:00 (EDT)")
    w = pd.Timestamp("2024-01-15 12:00").tz_localize("Europe/Paris")
    check(w.utcoffset().total_seconds() == 3600, "Europe/Paris in January: +01:00")
    check(pd.to_datetime("2024-03-10T14:30:00").dayofweek == 6, "to_datetime: 2024-03-10 is a Sunday")
    check((pd.Timestamp("2024-03-01") - pd.Timestamp("2024-02-01")).days == 29, "timedelta: February 2024 has 29 days")
def strings_missing():
    s = pd.Series(["  QNX ", "neutrino", None, "rtos"])
    check(s.str.strip().str.upper().tolist()[:2] == ["QNX", "NEUTRINO"], "str.strip().upper()")
    check(s.isna().sum() == 1 and s.fillna("-").iloc[2] == "-", "missing value: isna, fillna")
    n = pd.Series([1.0, np.nan, 3.0])
    check(n.mean() == 2.0 and n.interpolate().iloc[1] == 2.0, "NaN skipped by mean; interpolated")
    cat = pd.Categorical(["lo", "hi", "lo"], categories=["lo", "hi"], ordered=True)
    check(cat.codes.tolist() == [0, 1, 0] and cat.max() == "hi", "ordered categorical")
def csv():
    df.to_csv("t.csv", index=False)
    back = pd.read_csv("t.csv")
    check(back.equals(df), "to_csv / read_csv round trip")
    j = pd.read_json(_io.StringIO(df.to_json(orient="records")))
    check(j["sales"].sum() == 210, "to_json / read_json")
for n, f in (("groupby", groupby), ("merge", merge), ("timeseries", timeseries), ("strings/missing", strings_missing), ("csv", csv)):
    run(n, f)
sys.exit(1 if fails else 0)
P
"$PY" t.py; rc=$?
cd /; rm -rf "$T"
[ $rc -eq 0 ] && { echo "PASS pandas"; exit 0; }
echo "FAIL pandas (rc=$rc)"; exit 1
