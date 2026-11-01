#!/bin/sh
# CUPS end to end on loopback: a private cupsd (its own cupsd.conf and
# cups-files.conf in a scratch directory, listening on 127.0.0.1:<port> and
# a UNIX socket; nothing of the system's CUPS setup is touched). Then, with
# the command-line tools (libcups underneath):
#  - lpadmin creates a raw queue printing to a file (file: device),
#    lpstat -p/-v/-a report it idle, enabled and accepting;
#  - lp prints two text files as one job: it completes and the output
#    file is the two inputs concatenated, byte for byte; lpstat -W completed
#    lists the job (a raw queue runs no filter for a single uncompressed
#    file, and the file: device is then never opened -- upstream behaviour;
#    with two files CUPS runs its gziptoany copy filter for each);
#  - a held job (lp -H hold) is listed as held and removed by cancel;
#  - cupsreject/cupsaccept and cupsdisable/cupsenable change the queue state;
#  - lpoptions sets and reads back a default option;
#  - ipptool checks the queue's IPP attributes (name, state, info,
#    location ...) and the completed job over IPP (HTTP on 127.0.0.1);
#  - cupsctl reads the server settings.
# cupsd is stopped on every exit path.
echo "== cups =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
T=$(mktemp -d /tmp/qnxport-cups.XXXXXX) || { echo "FAIL cups (mktemp)"; exit 1; }
CPID=
stop_cupsd() {
  [ -n "$CPID" ] || return 0
  kill $CPID 2>/dev/null
  i=0; while kill -0 $CPID 2>/dev/null && [ $i -lt 10 ]; do sleep 1; i=$((i+1)); done
  kill -9 $CPID 2>/dev/null; wait $CPID 2>/dev/null
  CPID=
}
cleanup() { stop_cupsd; cd /; rm -rf "$T"; }
trap cleanup EXIT
trap 'exit 1' INT TERM HUP
cd "$T" || exit 1
chmod 755 "$T"
mkdir -p etc/ppd spool/tmp cache state log run out
chmod 1777 out spool/tmp

PORT=$((16310 + $$ % 300))
cat > etc/cups-files.conf <<EOF
ServerRoot $T/etc
RequestRoot $T/spool
TempDir $T/spool/tmp
CacheDir $T/cache
StateDir $T/state
AccessLog $T/log/access_log
ErrorLog $T/log/error_log
PageLog $T/log/page_log
FileDevice Yes
SystemGroup root
EOF
cat > etc/cupsd.conf <<EOF
LogLevel debug
Listen 127.0.0.1:$PORT
Listen $T/run/cups.sock
Browsing No
WebInterface No
DefaultAuthType None
ServerName 127.0.0.1
<Location />
  Order allow,deny
  Allow 127.0.0.1
  Allow @LOCAL
</Location>
<Policy default>
  <Limit All>
    Order deny,allow
  </Limit>
</Policy>
EOF
chmod 644 etc/cupsd.conf etc/cups-files.conf

cupsd -f -c $T/etc/cupsd.conf -s $T/etc/cups-files.conf > cupsd.out 2>&1 &
CPID=$!
CUPS_SERVER=$T/run/cups.sock; export CUPS_SERVER
i=0; while [ ! -S $T/run/cups.sock ] && [ $i -lt 60 ]; do kill -0 $CPID 2>/dev/null || break; sleep 0.5; i=$((i+1)); done
if [ -S $T/run/cups.sock ] && lpstat -r 2>&1 | grep -q 'is running'; then
  ok "cupsd running ($(lpstat -r 2>&1))"
else
  bad "cupsd did not start: $(cat cupsd.out | head -5) $(tail -5 log/error_log 2>/dev/null)"
fi

lpadmin -p qnxq -E -v file://$T/out/out.prn -D "QNX test queue" -L "QEMU" > lpadmin.log 2>&1
rc=$?
[ $rc -eq 0 ] && ok "lpadmin: raw queue qnxq on file://$T/out/out.prn" || bad "lpadmin rc=$rc: $(cat lpadmin.log)"
lpstat -p qnxq 2>&1 | grep -q 'printer qnxq is idle.*enabled' && ok "lpstat -p: qnxq idle, enabled" || bad "lpstat -p: $(lpstat -p qnxq 2>&1)"
lpstat -v qnxq 2>&1 | grep -q "device for qnxq: .*$T/out/out.prn" && ok "lpstat -v: device URI" || bad "lpstat -v: $(lpstat -v qnxq 2>&1)"
lpstat -a qnxq 2>&1 | grep -q 'qnxq accepting requests' && ok "lpstat -a: accepting" || bad "lpstat -a: $(lpstat -a qnxq 2>&1)"

i=0; while [ $i -lt 300 ]; do echo "CUPS on QNX, line $i"; i=$((i+1)); done > in.txt
printf 'second document\n' > in2.txt
cat in.txt in2.txt > expect.prn
r=$(lp -d qnxq -t "qnx-job" in.txt in2.txt 2>&1)
echo "  info lp: $r"
JOB=$(echo "$r" | sed -n 's/^request id is \(qnxq-[0-9]*\).*/\1/p')
[ -n "$JOB" ] && ok "lp: job $JOB queued (2 files)" || bad "lp: $r"
i=0; while ! cmp -s expect.prn out/out.prn && [ $i -lt 60 ]; do sleep 0.5; i=$((i+1)); done
cmp -s expect.prn out/out.prn && ok "job printed: output file is both documents byte for byte" \
  || bad "output: $(ls -l out 2>&1 | tail -n +2) $(grep -i 'error\|fail' log/error_log | tail -3)"
i=0; while ! lpstat -W completed -o qnxq 2>/dev/null | grep -q "^$JOB " && [ $i -lt 20 ]; do sleep 0.5; i=$((i+1)); done
lpstat -W completed -o qnxq 2>/dev/null | grep -q "^$JOB " && ok "lpstat -W completed lists $JOB" || bad "completed jobs: $(lpstat -W completed -o 2>&1)"

r=$(lp -d qnxq -H hold in.txt 2>&1)
HJOB=$(echo "$r" | sed -n 's/^request id is \(qnxq-[0-9]*\).*/\1/p')
lpstat -o qnxq 2>&1 | grep -q "^$HJOB " && ok "held job $HJOB listed as pending" || bad "held job: $r / $(lpstat -o 2>&1)"
cancel $HJOB 2>&1 && ! lpstat -o qnxq 2>&1 | grep -q "^$HJOB " && ok "cancel $HJOB" || bad "cancel: $(lpstat -o 2>&1)"

cupsreject -r "maintenance" qnxq && lpstat -a qnxq 2>&1 | grep -q 'not accepting' && ok "cupsreject: not accepting" || bad "cupsreject: $(lpstat -a qnxq 2>&1)"
lp -d qnxq in.txt > rej.log 2>&1 && bad "a rejecting queue took a job" || ok "rejecting queue refuses lp"
cupsaccept qnxq && lpstat -a qnxq 2>&1 | grep -q 'qnxq accepting' && ok "cupsaccept: accepting again" || bad "cupsaccept"
cupsdisable qnxq && lpstat -p qnxq 2>&1 | grep -q 'disabled' && ok "cupsdisable: stopped" || bad "cupsdisable: $(lpstat -p qnxq 2>&1)"
cupsenable qnxq && lpstat -p qnxq 2>&1 | grep -q 'enabled' && ok "cupsenable: enabled" || bad "cupsenable: $(lpstat -p qnxq 2>&1)"

HOME=$T; export HOME
lpoptions -d qnxq -o media=A4 > /dev/null 2>&1
lpoptions -p qnxq 2>&1 | grep -q 'media=A4' && ok "lpoptions: media=A4 stored" || bad "lpoptions: $(lpoptions -p qnxq 2>&1)"
lpstat -d 2>&1 | grep -q 'system default destination: qnxq' && ok "lpstat -d: qnxq is the default" || bad "lpstat -d: $(lpstat -d 2>&1)"

# An ipptool test of our own (CUPS's get-printer-attributes.test expects an
# IPP Everywhere printer's media-col-default, which a raw queue lacks; and a
# 2-document job repeats the per-document attributes, which ipptool -t
# flags, so only job-level ones are requested).
cat > qnxq.test <<'EOT'
{
  NAME "Get-Printer-Attributes of the QNX queue"
  OPERATION Get-Printer-Attributes
  GROUP operation-attributes-tag
  ATTR charset attributes-charset utf-8
  ATTR naturalLanguage attributes-natural-language en
  ATTR uri printer-uri $uri
  ATTR keyword requested-attributes all
  STATUS successful-ok
  EXPECT printer-name OF-TYPE name WITH-VALUE "qnxq"
  EXPECT printer-state OF-TYPE enum WITH-VALUE 3
  EXPECT printer-is-accepting-jobs OF-TYPE boolean WITH-VALUE true
  EXPECT printer-info OF-TYPE text WITH-VALUE "QNX test queue"
  EXPECT printer-location OF-TYPE text WITH-VALUE "QEMU"
  EXPECT printer-uri-supported OF-TYPE uri
  EXPECT ipp-versions-supported OF-TYPE keyword WITH-VALUE "2.0"
}
{
  NAME "Get-Job-Attributes of the printed job"
  OPERATION Get-Job-Attributes
  GROUP operation-attributes-tag
  ATTR charset attributes-charset utf-8
  ATTR naturalLanguage attributes-natural-language en
  ATTR uri printer-uri $uri
  ATTR integer job-id $jobid
  ATTR keyword requested-attributes job-state,number-of-documents,job-printer-uri
  STATUS successful-ok
  EXPECT job-state OF-TYPE enum WITH-VALUE 9
  EXPECT number-of-documents OF-TYPE integer WITH-VALUE 2
  EXPECT job-printer-uri OF-TYPE uri WITH-VALUE "/:[0-9]+.printers.qnxq/"
}
EOT
ipptool -t -d jobid=${JOB#qnxq-} ipp://127.0.0.1:$PORT/printers/qnxq qnxq.test > ipptool.log 2>&1; rc=$?
n=$(grep -c '\[PASS\]' ipptool.log)
[ $rc -eq 0 ] && [ "$n" -eq 2 ] && ok "ipptool over IPP (HTTP on 127.0.0.1): printer attributes and the completed 2-document job, 2 tests PASS" \
  || { bad "ipptool rc=$rc, $n passed:"; grep -v '^ *$' ipptool.log | head -8 | sed 's/^/       /'; }

cupsctl 2>&1 | grep -q '_remote_admin=0' && ok "cupsctl reads the server settings" || bad "cupsctl: $(cupsctl 2>&1 | head -3)"

lpadmin -x qnxq && ! lpstat -p qnxq > /dev/null 2>&1 && ok "lpadmin -x: queue removed" || bad "lpadmin -x"
stop_cupsd
grep -q 'Scheduler shutting down' log/error_log && ok "cupsd shut down on SIGTERM" || bad "cupsd shutdown: $(tail -3 log/error_log)"

cleanup; trap - EXIT
[ $fail -eq 0 ] && { echo "PASS cups"; exit 0; }
echo "FAIL cups ($fail check(s) failed)"; exit 1
