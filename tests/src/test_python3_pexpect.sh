#!/bin/sh
# python3-pexpect and python3-ptyprocess: pexpect drives an interactive
# /bin/sh through a pseudo terminal -- sets a prompt, runs commands and
# reads their output, sees the terminal size it set, sends Ctrl-C to a
# sleeping child, gets the exit status -- and pexpect.run() returns a
# command's output and status; ptyprocess spawns a child on a pty, reads
# and writes it, and reports isatty() inside it.
echo "== python3-pexpect =="
PY=${QNX_PORT_PYTHON:-/usr/local/bin/python3}; [ -x "$PY" ] || PY=python3
# No __pycache__ left in the installation's site-packages (docs/porting-hazards.md 183)
export PYTHONDONTWRITEBYTECODE=1
T=$(mktemp -d "${TMPDIR:-/tmp}/qnxport-pexpect.XXXXXX") || { echo "FAIL python3-pexpect (no temporary directory)"; exit 1; }
trap 'cd /; rm -rf "$T"' EXIT
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
        import traceback; traceback.print_exc(limit=4, file=sys.stdout)
        check(False, "%s: %r" % (name, e))
import pexpect, ptyprocess
info("pexpect %s, ptyprocess %s" % (pexpect.__version__, ptyprocess.__version__))
def shell():
    c = pexpect.spawn("/bin/sh", encoding="utf-8", timeout=20, dimensions=(30, 100), env=dict(os.environ, PS1="$ "))  # whole environment: python3 needs LD_LIBRARY_PATH
    c.sendline("PS1='QNXP> '")
    c.expect_exact("QNXP> "); c.expect_exact("QNXP> ")
    c.sendline("echo $((6 * 7))")
    c.expect(r"\r\n(\d+)\r\n"); check(c.match.group(1) == "42", "shell arithmetic echoed through the pty")
    c.expect_exact("QNXP> ")
    # QNX's stty has no "size" operand: the child asks with Python instead
    c.sendline('%s -c "import os; s = os.get_terminal_size(); print(\'SZ\', s.lines, s.columns)"' % sys.executable)
    i = c.expect([r"SZ (\d+) (\d+)", pexpect.TIMEOUT], timeout=20)
    check(i == 0 and c.match.groups() == ("30", "100"), "child sees the terminal size 30x100")
    c.expect_exact("QNXP> ")
    c.sendline("sleep 30")
    import time; time.sleep(1)
    c.sendintr()
    c.expect_exact("QNXP> ", timeout=10); check(True, "Ctrl-C interrupts a sleeping child")
    c.sendline("exit 3")
    c.expect(pexpect.EOF); c.close()
    check(c.exitstatus == 3, "exit status 3 from the shell")
run("shell", shell)
def runner():
    out, st = pexpect.run("/bin/sh -c 'echo hello; exit 5'", withexitstatus=True, timeout=20)
    check(out.strip() == b"hello" and st == 5, "pexpect.run(): output and exit status")
run("run", runner)
def timeout():
    c = pexpect.spawn("/bin/sh", ["-c", "sleep 10"], timeout=1)
    try:
        c.expect("never"); check(False, "timeout raises TIMEOUT")
    except pexpect.TIMEOUT:
        check(True, "expect() times out with TIMEOUT")
    c.terminate(force=True)
run("timeout", timeout)
def pty():
    p = ptyprocess.PtyProcessUnicode.spawn([sys.executable, "-c", "import sys, os; print('tty', os.isatty(0), os.isatty(1)); print(input()[::-1])"])
    p.write("onrtuen\n")
    buf = ""
    while True:
        try: buf += p.read(1024)
        except EOFError: break
    p.wait()
    check("tty True True" in buf, "ptyprocess: the child's stdin and stdout are terminals")
    check("neutrno" in buf.replace("\r", ""), "ptyprocess: written line read back by the child")
    check(p.exitstatus == 0, "ptyprocess: child exit status 0")
run("ptyprocess", pty)
sys.exit(1 if fails else 0)
P
"$PY" t.py; rc=$?
[ $rc -eq 0 ] && { echo "PASS python3-pexpect"; exit 0; }
echo "FAIL python3-pexpect (rc=$rc)"; exit 1
