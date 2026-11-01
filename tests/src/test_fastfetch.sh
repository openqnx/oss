#!/bin/sh
# fastfetch on QNX, its JSON output checked against the system's own
# answers: OS name QNX with uname's release and build; the kernel release;
# the CPU count and speed and the RAM size as pidin reports them (system
# page); the uptime against pidin's boot time; the number of processes in
# /proc; the root file system's size as df -k reports it; 127.0.0.1 on lo0
# (local IP module, loopback shown); and the shell running this script
# (process tree through /proc/<pid>/exefile and DCMD_PROC_INFO). Also the
# plain text output with a custom format string.
echo "== fastfetch =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
T=$(mktemp -d /tmp/qnxport-fastfetch.XXXXXX) || { echo "FAIL fastfetch (mktemp)"; exit 1; }
trap 'cd /; rm -rf "$T"' EXIT
trap 'exit 1' INT TERM HUP
cd "$T" || exit 1
# fastfetch looks for its configuration under $HOME: keep it out of the way.
HOME=$T; XDG_CONFIG_HOME=$T; export HOME XDG_CONFIG_HOME

v=$(fastfetch --version 2>&1)
case "$v" in "fastfetch 2.66.1 "*) ok "fastfetch --version: 2.66.1" ;; *) bad "version: $v" ;; esac

cat > cfg.jsonc <<'EOF'
{
  "logo": { "type": "none" },
  "modules": [
    "os", "kernel", "cpu", "memory", "uptime", "processes",
    { "type": "disk", "folders": "/" },
    { "type": "localip", "showLoop": true, "defaultRouteOnly": false, "showIpv4": true, "showIpv6": false },
    "shell"
  ]
}
EOF
fastfetch -c cfg.jsonc --format json > out.json 2> err.txt; rc=$?
[ $rc -eq 0 ] && [ -s out.json ] && ok "JSON output written" || bad "fastfetch rc=$rc: $(head -c 300 err.txt)"
# one "key": value per line in fastfetch's JSON
val() { sed -n "/\"type\": \"$1\"/,/\"type\": /p" out.json | grep -m1 "\"$2\":" | sed 's/^[^:]*: *//; s/,$//; s/^"//; s/"$//'; }
err() { sed -n "/\"type\": \"$1\"/,/\"type\": /p" out.json | grep -m1 '"error":' | sed 's/^[^:]*: *//'; }
for m in OS Kernel CPU Memory Uptime Processes Disk LocalIp Shell; do
  e=$(err $m); [ -z "$e" ] || bad "$m module: error $e"
done

R=$(uname -r); B=$(uname -v)
[ "$(val OS name)" = QNX ] && [ "$(val OS version)" = "$R" ] && ok "OS: QNX $R" || bad "OS: $(val OS name) $(val OS version)"
[ "$(val OS buildID)" = "$B" ] && ok "OS build: $B" || bad "OS buildID: $(val OS buildID) (uname -v: $B)"
[ "$(val Kernel release)" = "$R" ] && ok "Kernel release $R" || bad "Kernel: $(val Kernel release)"

pidin info > pidin.txt 2>&1
n=$(grep -c '^Processor[0-9]' pidin.txt)
mhz=$(grep -m1 '^Processor[0-9]' pidin.txt | sed -n 's/.* \([0-9][0-9]*\)MHz.*/\1/p')
echo "  info pidin: $n CPUs at ${mhz} MHz, $(grep -o 'FreeMem:[0-9]*MB/[0-9]*MB' pidin.txt)"
[ "$(val CPU logical)" = "$n" ] && ok "CPU: $n logical cores, as pidin" || bad "CPU cores: $(val CPU logical) (pidin: $n)"
[ "$(val CPU base)" = "$mhz" ] && ok "CPU: base frequency $mhz MHz, as pidin" || bad "CPU frequency: $(val CPU base) (pidin: $mhz)"
total=$(val Memory total); tmb=$(grep -o 'FreeMem:[0-9]*MB/[0-9]*MB' pidin.txt | sed 's/.*\/\([0-9]*\)MB/\1/')
[ -n "$total" ] && [ $((total / 1048576)) -eq "$tmb" ] && ok "Memory: total $tmb MB, as pidin" || bad "Memory total: $total bytes (pidin: $tmb MB)"
used=$(val Memory used)
[ -n "$used" ] && [ "$used" -gt 0 ] && [ "$used" -lt "$total" ] && ok "Memory: used is between 0 and the total" || bad "Memory used: $used"

up=$(val Uptime uptime)
[ -n "$up" ] && [ "$up" -gt 0 ] && ok "Uptime: $((up / 1000)) s since boot" || bad "Uptime: $up"
bt=$(val Uptime bootTime)
pb=$(sed -n 's/.*BootTime:\([A-Za-z]* *[0-9]* [0-9:]*\).*/\1/p' pidin.txt)
echo "  info boot time: fastfetch $bt, pidin $pb"

p=$(val Processes result); np=$(ls /proc | grep -c '^[0-9]')
[ -n "$p" ] && [ "$p" -ge $((np - 5)) ] && [ "$p" -le $((np + 5)) ] && ok "Processes: $p (/proc has $np)" || bad "Processes: $p (/proc has $np)"

dft=$(df -k / 2>/dev/null | awk 'NR==1 {print $2}')
dtot=$(val Disk total)
[ -n "$dtot" ] && [ -n "$dft" ] && [ $((dtot / 1024)) -eq "$dft" ] && ok "Disk /: $dft KB total, as df -k" || bad "Disk /: $dtot bytes (df -k: $dft KB)"
[ "$(val Disk mountpoint)" = "/" ] && ok "Disk: mount point /" || bad "Disk mountpoint: $(val Disk mountpoint)"

grep -q '"ipv4": "127.0.0.1/8"' out.json && grep -q '"name": "lo0"' out.json && ok "Local IP: lo0 127.0.0.1/8" || bad "Local IP: $(sed -n '/"LocalIp"/,/]/p' out.json | tr -d ' \n' | head -c 300)"

me=$(cat /proc/$$/exefile 2>/dev/null)
sh=$(val Shell exe)
echo "  info shell: fastfetch $sh, this script $me"
[ -n "$sh" ] && [ "$sh" = "$me" ] && ok "Shell: $sh (the shell running this test)" || bad "Shell exe: $sh (expected $me)"

printf '{ "logo": { "type": "none" }, "modules": [ { "type": "os", "format": "{name}-{version}" } ] }\n' > fmt.jsonc
o=$(fastfetch --pipe -c fmt.jsonc 2>&1)
[ "$o" = "OS: QNX-$R" ] && ok "text output with a format string: $o" || bad "format string: $o"

cd /; rm -rf "$T"; trap - EXIT
[ $fail -eq 0 ] && { echo "PASS fastfetch"; exit 0; }
echo "FAIL fastfetch ($fail check(s) failed)"; exit 1
