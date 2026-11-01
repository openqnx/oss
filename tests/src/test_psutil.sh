#!/bin/sh
# psutil on QNX (the _psutil_qnx module from qnx-ports, ports/psutil):
# answers checked against what the test knows about itself -- its own pid,
# parent, uid/gid, executable and command line, a thread it starts and a
# 64 MiB buffer it allocates (num_threads, memory_info), a child it starts,
# stops and waits for; the system's CPU count against os.cpu_count(), boot
# time against the clock, memory totals, the loopback interface, disk usage
# of / against os.statvfs, and NoSuchProcess for a pid that does not exist.
echo "== psutil =="
PY=${QNX_PORT_PYTHON:-/usr/local/bin/python3}; [ -x "$PY" ] || PY=python3
T=$(mktemp -d "${TMPDIR:-/tmp}/qnxport-psutil.XXXXXX") || { echo "FAIL psutil (no temporary directory)"; exit 1; }
cd "$T" || exit 1
cat > t.py <<'P'
import sys, os, time, threading, subprocess
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
import psutil
info("psutil %s on %s" % (psutil.__version__, sys.platform))
def system():
    n = psutil.cpu_count()
    check(n == os.cpu_count() and n >= 1, "cpu_count() = os.cpu_count() = %s" % n)
    check(psutil.cpu_count(logical=False) == n, "cpu_count(logical=False) = %s" % n)
    bt = psutil.boot_time(); up = time.time() - bt
    info("booted %.0f s ago" % up)
    check(1.5e9 < bt <= time.time() and up > 0, "boot_time() is in the past")
    vm = psutil.virtual_memory(); info("memory: total %d MiB, available %d MiB" % (vm.total >> 20, vm.available >> 20))
    check(vm.total > 64 << 20 and 0 < vm.available <= vm.total and 0 <= vm.percent <= 100, "virtual_memory(): total > 64 MiB, 0 < available <= total")
    check(psutil.swap_memory().total == 0, "swap_memory(): QNX has no swap")
    fr = psutil.cpu_freq(percpu=True); info("cpu_freq: %s MHz" % [f.current for f in fr])
    # the system page's CPU speed: 0 on some VMs (QEMU's aarch64 virt board)
    check(len(fr) == n and all(f.current >= 0 for f in fr), "cpu_freq(): one entry per CPU")
    ct = psutil.cpu_times(); info("cpu_times: %s" % (ct,))
    check(ct.idle >= 0 and ct.system >= 0 and ct.idle + ct.system <= n * up * 1.05 + 1, "cpu_times(): idle + busy within uptime x CPUs")
    p = psutil.cpu_percent(interval=0.5); check(0.0 <= p <= 100.0, "cpu_percent(interval=0.5) = %.1f" % p)
    du = psutil.disk_usage("/"); sv = os.statvfs("/")
    check(du.total == sv.f_blocks * sv.f_frsize and du.free <= du.total, "disk_usage('/') total = statvfs blocks x frsize")
def network():
    addrs = psutil.net_if_addrs(); info("interfaces: %s" % sorted(addrs))
    lo = [n for n, a in addrs.items() if any(x.address == "127.0.0.1" for x in a)]
    check(len(lo) == 1, "net_if_addrs(): 127.0.0.1 on %s" % (lo[0] if lo else "?"))
    io = psutil.net_io_counters(pernic=True)
    check(lo and lo[0] in io and io[lo[0]].bytes_sent >= 0, "net_io_counters(): counters for the loopback")
    st = psutil.net_if_stats()
    check(lo and lo[0] in st and st[lo[0]].isup and st[lo[0]].mtu > 0, "net_if_stats(): loopback is up, MTU %s" % (st[lo[0]].mtu if lo and lo[0] in st else "?"))
def me():
    p = psutil.Process()
    check(p.pid == os.getpid() and p.ppid() == os.getppid(), "Process(): pid and ppid match os")
    check(p.uids().real == os.getuid() and p.uids().effective == os.geteuid() and p.gids().real == os.getgid(), "uids()/gids() match os")
    exe = p.exe(); info("exe %s, name %s" % (exe, p.name()))
    check(os.path.realpath(exe) == os.path.realpath(sys.executable) or os.path.basename(exe).startswith("python"), "exe() is the Python interpreter")
    check(p.name() == os.path.basename(exe), "name() is the executable's base name")
    cl = p.cmdline(); info("cmdline %s" % cl)
    check(len(cl) >= 2 and cl[-2:] == ["t.py", "qnx-arg"], "cmdline() ends with t.py qnx-arg")
    ctime = p.create_time()
    check(time.time() - 120 < ctime <= time.time() + 1, "create_time() within the last 2 minutes")
    # the thread asking is the one reported: on QNX it is REPLY-blocked on
    # procnto while its own devctl() is answered
    check(p.status() in ("running", "ready", "reply"), "status() of itself: %s" % p.status())
    n0 = p.num_threads(); ev = threading.Event(); th = threading.Thread(target=ev.wait); th.start()
    n1 = p.num_threads(); tl = p.threads(); ev.set(); th.join()
    check(n1 == n0 + 1 and len(tl) == n1, "num_threads() %d -> %d with one more thread; threads() lists %d" % (n0, n1, len(tl)))
    m0 = p.memory_info(); buf = bytearray(64 << 20); buf[::4096] = b"\1" * len(buf[::4096]); m1 = p.memory_info()
    info("rss %d -> %d KiB, vms %d KiB" % (m0.rss >> 10, m1.rss >> 10, m1.vms >> 10))
    check(m1.rss - m0.rss >= 56 << 20 and m1.vms >= m1.rss, "memory_info(): rss grows by ~64 MiB for a 64 MiB buffer")
    del buf
    t = p.cpu_times(); x = 0
    for i in range(2000000): x += i
    t2 = p.cpu_times(); check(t2.user + t2.system > t.user + t.system, "cpu_times() grows while computing")
    check(0 < p.memory_percent() < 100, "memory_percent()")
def child():
    c = subprocess.Popen([sys.executable, "-c", "import time; time.sleep(30)"])
    try:
        # QNX's exec() gives the process a new start time; until the child
        # has exec'd, a Process for it would be "reused" afterwards. Wait
        # for the child's own command line.
        for i in range(100):
            try:
                if "time.sleep(30)" in " ".join(psutil.Process(c.pid).cmdline()): break
            except psutil.Error: pass
            time.sleep(0.05)
        pc = psutil.Process(c.pid)
        check(pc.ppid() == os.getpid(), "child's ppid() is this process")
        check(c.pid in psutil.pids() and psutil.pid_exists(c.pid), "child in pids(), pid_exists()")
        check(c.pid in [x.pid for x in psutil.Process().children()], "Process().children() lists it")
        pc.terminate(); rc = pc.wait(timeout=10)
        check(rc is not None and not psutil.pid_exists(c.pid), "terminate() + wait(): exit status %s, gone" % rc)
    finally:
        if c.poll() is None: c.kill()
def missing():
    pid = max(psutil.pids()) + 100000
    try: psutil.Process(pid); check(False, "Process(nonexistent) raises NoSuchProcess")
    except psutil.NoSuchProcess: check(True, "Process(nonexistent) raises NoSuchProcess")
    check(1 in psutil.pids() and os.getpid() in psutil.pids(), "pids() has procnto (1) and this process")
    names = set()
    for pr in psutil.process_iter(["name"]): names.add(pr.info["name"])
    info("%d processes" % len(psutil.pids()))
    check(any(n and n.startswith("python") for n in names), "process_iter(['name']) finds python")
for n, f in (("system", system), ("network", network), ("process", me), ("child", child), ("missing", missing)):
    run(n, f)
sys.exit(1 if fails else 0)
P
"$PY" t.py qnx-arg; rc=$?
cd /; rm -rf "$T"
[ $rc -eq 0 ] && { echo "PASS psutil"; exit 0; }
echo "FAIL psutil (rc=$rc)"; exit 1
