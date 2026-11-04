#!/bin/sh
# pytest plugins: a small suite run with pytest -n 2 (pytest-xdist, its
# workers started through execnet): both workers gw0 and gw1 run tests and
# the summary counts exactly; the mocker fixture (pytest-mock) patches and
# spies; time-machine moves the clock to a fixed instant (time.time,
# datetime.now, ticking off) and back; flaky reruns a test that fails on its
# first run and passes on the second. execnet is also used directly: a
# subprocess gateway runs code and answers over a channel.
echo "== python3-pytest-plugins =="
PY=${QNX_PORT_PYTHON:-/usr/local/bin/python3}; [ -x "$PY" ] || PY=python3
# No __pycache__ left in the installation's site-packages (docs/porting-hazards.md 183)
export PYTHONDONTWRITEBYTECODE=1
T=$(mktemp -d "${TMPDIR:-/tmp}/qnxport-pytestpl.XXXXXX") || { echo "FAIL python3-pytest-plugins (no temporary directory)"; exit 1; }
trap 'cd /; rm -rf "$T"' EXIT
cd "$T" || exit 1
fail=0
ok()  { echo "  ok   $1"; }
bad() { echo "  FAIL $1"; fail=1; }
mkdir suite
cat > suite/test_suite.py <<'P'
import os, time, datetime, subprocess
import pytest, time_machine
from flaky import flaky

def mark():
    open(os.path.join(os.environ["QNXP_OUT"], "%s-%d" % (os.environ.get("PYTEST_XDIST_WORKER", "none"), os.getpid())), "a").write("x")

@pytest.mark.parametrize("n", range(8))
def test_square(n):
    mark(); time.sleep(0.2)
    assert n * n == [0, 1, 4, 9, 16, 25, 36, 49][n]

def test_mocker_patch(mocker):
    mark()
    m = mocker.patch("os.getcwd", return_value="/qnx")
    assert os.getcwd() == "/qnx" and m.call_count == 1

def test_mocker_spy(mocker):
    mark()
    spy = mocker.spy(subprocess, "list2cmdline")
    assert subprocess.list2cmdline(["a", "b c"]) == 'a "b c"'
    spy.assert_called_once_with(["a", "b c"])
    assert spy.spy_return == 'a "b c"'

def test_time_machine():
    mark()
    with time_machine.travel(datetime.datetime(2000, 1, 1, 12, 0, tzinfo=datetime.timezone.utc), tick=False):
        assert time.time() == 946728000.0
        assert datetime.datetime.now(datetime.timezone.utc).isoformat() == "2000-01-01T12:00:00+00:00"
    assert time.time() > 1.7e9

def test_time_machine_fixture(time_machine):
    mark()
    time_machine.move_to(datetime.datetime(1979, 5, 27, tzinfo=datetime.timezone.utc), tick=False)
    assert time.gmtime().tm_year == 1979 and time.gmtime().tm_mon == 5
    time_machine.shift(datetime.timedelta(days=5))
    assert time.gmtime().tm_mday == 1 and time.gmtime().tm_mon == 6

@flaky(max_runs=3, min_passes=1)
def test_flaky_second_time():
    mark()
    f = os.path.join(os.environ["QNXP_OUT"], "flaky-count")
    n = int(open(f).read()) if os.path.exists(f) else 0
    open(f, "w").write(str(n + 1))
    assert n >= 1, "fails on the first run"
P
mkdir out
QNXP_OUT="$T/out" "$PY" -m pytest -p no:cacheprovider -n 2 -v suite > log.txt 2>&1; rc=$?
cat log.txt | sed 's/^/  | /' | tail -n 30
[ $rc -eq 0 ] && ok "pytest -n 2 exits 0" || bad "pytest -n 2 exits $rc"
grep -q "created: 2/2 workers" log.txt && ok "pytest-xdist created 2 workers" || bad "pytest-xdist created 2 workers"
grep -Eq "(^|= )13 passed" log.txt && ok "13 passed (8 parametrized, 2 mocker, 2 time-machine, 1 flaky)" || bad "13 passed"
ls out | grep -q '^gw0-' && ls out | grep -q '^gw1-' && ok "tests ran on both gw0 and gw1" || bad "tests ran on both gw0 and gw1 ($(ls out | tr '\n' ' '))"
[ "$(cat out/flaky-count 2>/dev/null)" = 2 ] && ok "flaky reran the failing test once (2 runs)" || bad "flaky reran the failing test once (runs: $(cat out/flaky-count 2>/dev/null))"
grep -q "test_flaky_second_time passed 1 out of the required 1 times" log.txt && ok "flaky report names the rerun test" || bad "flaky report names the rerun test"
cat > gw.py <<'P'
import execnet
gw = execnet.makegateway("popen")
ch = gw.remote_exec("import os\nx = channel.receive()\nchannel.send((sum(range(x)), os.getpid()))")
ch.send(101)
s, pid = ch.receive()
import os
print("execnet", s, pid != os.getpid())
gw.exit()
P
r=$("$PY" gw.py 2>&1)
[ "$r" = "execnet 5050 True" ] && ok "execnet: a popen gateway computes sum(range(101)) = 5050 in another process" || bad "execnet popen gateway ($r)"
[ $fail -eq 0 ] && { echo "PASS python3-pytest-plugins"; exit 0; }
echo "FAIL python3-pytest-plugins"; exit 1
