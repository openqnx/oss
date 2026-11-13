#!/bin/sh
# clinfo: what it prints about the OpenCL platforms the ICD loader finds.
#  - no OpenCL implementation (OCL_ICD_VENDORS names an empty directory):
#    "Number of platforms 0" and exit status 0 -- the documented result, not
#    an error -- and still the ICD loader's own properties, which clinfo
#    asks the loader for (clGetICDLoaderInfoOCLICD): "Khronos OpenCL ICD
#    Loader", "Khronos Group"; the same in --raw ("#PLATFORMS 0",
#    "[OCLICD/*] CL_ICDL_NAME ...") and --json ("platforms" : [ ]); --list
#    prints nothing;
#  - --version, --help and an unknown option (reported, then ignored);
#  - when pocl is installed (its pocl.icd in <prefix>/etc/OpenCL/vendors):
#    --list shows the platform "Portable Computing Language" with one
#    device, --raw gives CL_PLATFORM_NAME, CL_DEVICE_TYPE_CPU and a
#    CL_DEVICE_MAX_COMPUTE_UNITS of at least 1, --json names the platform,
#    and "--prop CL_DEVICE_TYPE" selects one property. Without pocl these are
#    skipped (reported as "info").
# Run the test itself in the background, its output to a log, and wait at
# most 170 s: nothing it leaves behind can hold the runner's output, and a
# hang ends as a FAIL with whatever the test printed so far.
if [ -z "$QNXPORT_INNER" ]; then
  QNXPORT_INNER=/tmp/qnxport-out.$$; export QNXPORT_INNER
  sh "$0" "$@" > "$QNXPORT_INNER" 2>&1 < /dev/null &
  qp=$!; qi=0
  while kill -0 $qp 2>/dev/null && [ $qi -lt 170 ]; do sleep 1; qi=$((qi+1)); done
  if kill -0 $qp 2>/dev/null; then
    for p in $(cat "$QNXPORT_INNER.pids" 2>/dev/null); do kill $p 2>/dev/null; done
    kill $qp 2>/dev/null; sleep 1; kill -KILL $qp 2>/dev/null
    cat "$QNXPORT_INNER"; echo "  FAIL watchdog: still running after 170 s -- stopped"
    echo "FAIL ${0##*/} (watchdog)"; rm -f "$QNXPORT_INNER" "$QNXPORT_INNER.pids"; exit 1
  fi
  wait $qp; rc=$?; cat "$QNXPORT_INNER"; rm -f "$QNXPORT_INNER" "$QNXPORT_INNER.pids"; exit $rc
fi
N=clinfo
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
L=/usr/local; [ -x "$L/bin/clinfo" ] || L=/data/oss/usr/local
QP=$QNXPORT_INNER.pids; : > "$QP"
stopall() {
  for p in $(cat "$QP" 2>/dev/null); do kill "$p" 2>/dev/null; done
  : > "$QP"; [ -n "$T" ] && [ -d "$T" ] && { cd /; rm -rf "$T"; }
}
trap stopall EXIT
trap 'exit 143' TERM
C=$L/bin/clinfo
[ -x "$C" ] || { echo "FAIL $N ($C is not installed)"; exit 1; }
T=$(mktemp -d /tmp/qnxport-clinfo.XXXXXX) || { echo "FAIL $N (mktemp)"; exit 1; }
cd "$T" || exit 1
mkdir empty
unset OCL_ICD_FILENAMES OPENCL_LAYERS OPENCL_LAYER_PATH

r=$("$C" --version 2>&1)
echo "$r" | grep -Eq '^clinfo version [0-9]+\.[0-9]+\.[0-9.]+$' && ok "--version: $r" || bad "--version: $r"
"$C" --help > help.txt 2>&1; rc=$?
[ $rc -eq 0 ] && grep -q '^Usage: clinfo \[options \.\.\.\]' help.txt && grep -q -e '--json' help.txt && grep -q -e '--list, -l' help.txt \
  && ok "--help: usage with --json and --list" || bad "--help rc=$rc: $(head -n 3 help.txt)"

# ---- no OpenCL implementation ----
OCL_ICD_VENDORS=$T/empty; export OCL_ICD_VENDORS
"$C" > human.txt 2> human.err; rc=$?
[ $rc -eq 0 ] && ok "no platform: exit status 0" || bad "no platform: exit status $rc: $(head -n 3 human.err)"
grep -Eq '^Number of platforms +0$' human.txt && ok "no platform: \"Number of platforms 0\"" || bad "no platform: $(head -n 2 human.txt)"
grep -Eq '^  ICD loader Name +Khronos OpenCL ICD Loader$' human.txt && grep -Eq '^  ICD loader Vendor +Khronos Group$' human.txt \
  && ok "ICD loader properties: Khronos OpenCL ICD Loader, Khronos Group" || bad "ICD loader properties: $(grep 'ICD loader' human.txt | head -n 3)"
grep -Eq '^  ICD loader Profile +OpenCL [0-9]+\.[0-9]+$' human.txt && ok "ICD loader profile: $(sed -n 's/^  ICD loader Profile *//p' human.txt)" \
  || bad "ICD loader profile: $(grep Profile human.txt)"
"$C" --raw > raw.txt 2>&1; rc=$?
[ $rc -eq 0 ] && grep -Eq '^#PLATFORMS +0$' raw.txt && grep -Eq '^\[OCLICD/\*\] +CL_ICDL_NAME +Khronos OpenCL ICD Loader$' raw.txt \
  && ok "--raw: #PLATFORMS 0 and [OCLICD/*] CL_ICDL_NAME" || bad "--raw rc=$rc: $(head -n 3 raw.txt)"
"$C" --json > json.txt 2>&1; rc=$?
[ $rc -eq 0 ] && grep -q '^{ "platforms" : \[ \], "icd_loader" : { "CL_ICDL_NAME" : "Khronos OpenCL ICD Loader", "CL_ICDL_VENDOR" : "Khronos Group"' json.txt \
  && ok "--json: empty platforms array and the icd_loader object" || bad "--json rc=$rc: $(head -c 200 json.txt)"
r=$("$C" --list 2>&1); rc=$?
[ $rc -eq 0 ] && [ -z "$r" ] && ok "--list: nothing to list" || bad "--list rc=$rc: $r"
"$C" --no-such-option > unk.txt 2>&1; rc=$?
[ $rc -eq 0 ] && grep -q 'ignoring unknown command-line parameter --no-such-option' unk.txt && grep -Eq '^Number of platforms +0$' unk.txt \
  && ok "an unknown option is reported and ignored" || bad "unknown option rc=$rc: $(head -n 2 unk.txt)"

# ---- with pocl, when it is installed ----
V=$L/etc/OpenCL/vendors
if [ -f "$V/pocl.icd" ]; then
  OCL_ICD_VENDORS=$V; export OCL_ICD_VENDORS
  POCL_CACHE_DIR=$T/cache; export POCL_CACHE_DIR
  "$C" --list > list.txt 2> list.err; rc=$?
  [ $rc -eq 0 ] && grep -q '^Platform #[0-9]*: Portable Computing Language$' list.txt && grep -Eq '^ .-- Device #0: ' list.txt \
    && ok "pocl: --list shows $(grep 'Portable' list.txt | head -n 1) and$(grep 'Device #0' list.txt | head -n 1 | sed 's/^ .--//')" \
    || bad "pocl --list rc=$rc: $(head -n 3 list.txt) $(head -n 3 list.err)"
  "$C" --raw > praw.txt 2> praw.err; rc=$?
  [ $rc -eq 0 ] && grep -Eq 'CL_PLATFORM_NAME +Portable Computing Language$' praw.txt && ok "pocl: --raw CL_PLATFORM_NAME" \
    || bad "pocl --raw rc=$rc: $(head -n 3 praw.txt) $(head -n 3 praw.err)"
  grep -Eq 'CL_DEVICE_TYPE +CL_DEVICE_TYPE_CPU$' praw.txt && ok "pocl: the device is CL_DEVICE_TYPE_CPU" || bad "pocl: $(grep 'CL_DEVICE_TYPE ' praw.txt | head -n 1)"
  cu=$(sed -n 's/.*CL_DEVICE_MAX_COMPUTE_UNITS  *\([0-9][0-9]*\)$/\1/p' praw.txt | head -n 1)
  [ -n "$cu" ] && [ "$cu" -ge 1 ] && ok "pocl: CL_DEVICE_MAX_COMPUTE_UNITS $cu" || bad "pocl: CL_DEVICE_MAX_COMPUTE_UNITS '$cu'"
  grep -Eq 'CL_DEVICE_OPENCL_C_VERSION +OpenCL C [0-9]' praw.txt && ok "pocl: $(sed -n 's/.*\(CL_DEVICE_OPENCL_C_VERSION\)  *\(.*\)/\1 \2/p' praw.txt | head -n 1)" \
    || bad "pocl: CL_DEVICE_OPENCL_C_VERSION"
  "$C" --json > pjson.txt 2>&1; rc=$?
  [ $rc -eq 0 ] && grep -q '"CL_PLATFORM_NAME" : "Portable Computing Language"' pjson.txt && grep -q '"CL_DEVICE_TYPE" : ' pjson.txt \
    && ok "pocl: --json has the platform and its device" || bad "pocl --json rc=$rc: $(head -c 200 pjson.txt)"
  n=$("$C" --raw --prop CL_DEVICE_TYPE 2>/dev/null | grep -c 'CL_DEVICE_TYPE')
  m=$("$C" --raw --prop CL_DEVICE_TYPE 2>/dev/null | grep -c 'CL_DEVICE_NAME')
  [ "$n" -ge 1 ] && [ "$m" -eq 0 ] && ok "pocl: --prop CL_DEVICE_TYPE prints that property only" || bad "pocl --prop: $n matching, $m other lines"
else
  info "pocl is not installed ($V/pocl.icd): platform and device output not checked"
fi
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
