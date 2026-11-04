#!/bin/sh
# cups-filters 1.28.17, the filters run the way cupsd runs them
# (filter job-id user title copies options [file], output on stdout), on a
# two-page PDF ("Page one", "Page two") made here and repaired by qpdf:
#  - pdftopdf (qpdf underneath): page-ranges=2 keeps only page 2,
#    number-up=2 puts both pages on one sheet, no options keeps 2 pages;
#  - pdftops (Poppler's pdftops): PostScript with %%Pages: 2;
#  - texttopdf: a text file to a one-page PDF with an embedded TrueType
#    font (fontconfig's monospace, DejaVu Sans Mono here);
#  - imagetopdf: a PPM image to a one-page PDF;
#  - driverless: a PPD generated from a live IPP Everywhere printer
#    (CUPS's ippeveprinter on 127.0.0.1, DNS-SD off), with the printer's
#    make/model and a PWG raster filter line;
#  - pdftoraster with that PPD: CUPS raster, 2 pages; rastertopdf turns it
#    back into a 2-page PDF.
# ippeveprinter (and the private D-Bus daemon it may need) are stopped on
# every exit path.
echo "== cups-filters =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
T=$(mktemp -d /tmp/qnxport-cupsfilters.XXXXXX) || { echo "FAIL cups-filters (mktemp)"; exit 1; }
EPID=; DPID=
cleanup() {
  [ -n "$DPID" ] && { kill $DPID 2>/dev/null; wait $DPID 2>/dev/null; DPID=; }
  if [ -n "$EPID" ]; then
    kill $EPID 2>/dev/null
    i=0; while kill -0 $EPID 2>/dev/null && [ $i -lt 10 ]; do sleep 1; i=$((i+1)); done
    kill -9 $EPID 2>/dev/null; wait $EPID 2>/dev/null; EPID=
  fi
  cd /; rm -rf "$T"
}
trap cleanup EXIT
trap 'echo "  FAIL interrupted"; exit 1' INT TERM HUP
cd "$T" || exit 1
F=/usr/local/lib/cups/filter
[ -x $F/pdftopdf ] || F=$(dirname "$(command -v pdftopdf 2>/dev/null || echo /x/x)")
filt() { name=$1; opts=$2; in=$3; out=$4; "$F/$name" 1 qnx "qnx-test" 1 "$opts" "$in" > "$out" 2> "$name.err"; }
npages() { qpdf --show-npages "$1" 2>/dev/null; }

cat > raw.pdf <<'PDF'
%PDF-1.4
1 0 obj << /Type /Catalog /Pages 2 0 R >> endobj
2 0 obj << /Type /Pages /Kids [3 0 R 4 0 R] /Count 2 >> endobj
3 0 obj << /Type /Page /Parent 2 0 R /MediaBox [0 0 595 842] /Contents 5 0 R /Resources << /Font << /F1 7 0 R >> >> >> endobj
4 0 obj << /Type /Page /Parent 2 0 R /MediaBox [0 0 595 842] /Contents 6 0 R /Resources << /Font << /F1 7 0 R >> >> >> endobj
5 0 obj << /Length 40 >>
stream
BT /F1 24 Tf 72 720 Td (Page one) Tj ET
endstream
endobj
6 0 obj << /Length 40 >>
stream
BT /F1 24 Tf 72 720 Td (Page two) Tj ET
endstream
endobj
7 0 obj << /Type /Font /Subtype /Type1 /BaseFont /Helvetica >> endobj
trailer << /Size 8 /Root 1 0 R >>
%%EOF
PDF
qpdf raw.pdf in.pdf 2>/dev/null
[ "$(npages in.pdf)" = 2 ] && ok "2-page input PDF (qpdf rebuilt its xref)" || bad "input PDF: $(qpdf --check in.pdf 2>&1 | tail -n 2)"

# --- pdftopdf ---
filt pdftopdf "" in.pdf all.pdf
n=$(npages all.pdf); [ "$n" = 2 ] && ok "pdftopdf: 2 pages through" || bad "pdftopdf: $n pages $(tail -n 3 pdftopdf.err)"
filt pdftopdf "page-ranges=2" in.pdf p2.pdf
n=$(npages p2.pdf); t=$(pdftotext p2.pdf - 2>/dev/null | tr -s '\n' ' ')
[ "$n" = 1 ] && echo "$t" | grep -q "Page two" && ! echo "$t" | grep -q "Page one" \
  && ok "pdftopdf page-ranges=2: only \"Page two\"" || bad "page-ranges: $n pages, text '$t' $(tail -n 3 pdftopdf.err)"
filt pdftopdf "number-up=2" in.pdf nup.pdf
n=$(npages nup.pdf); t=$(pdftotext nup.pdf - 2>/dev/null | tr -s '\n' ' ')
[ "$n" = 1 ] && echo "$t" | grep -q "Page one" && echo "$t" | grep -q "Page two" \
  && ok "pdftopdf number-up=2: both pages on one sheet" || bad "number-up: $n pages, '$t'"

# --- pdftops ---
filt pdftops "" in.pdf out.ps
head -c 10 out.ps | grep -q '^%!PS-Adobe' && grep -q '^%%Pages: 2' out.ps \
  && ok "pdftops: PostScript, %%Pages: 2" || bad "pdftops: $(head -c 40 out.ps) $(tail -n 3 pdftops.err)"

# --- texttopdf ---
printf 'Hello from texttopdf on QNX\nsecond line 12345\n' > in.txt
CHARSET=utf-8 filt texttopdf "" in.txt txt.pdf
# The text is drawn with an embedded subset of a TrueType font (glyph ids,
# no ToUnicode map, so pdftotext cannot read it back): check the page and
# the font that fontconfig chose.
qpdf --qdf --object-streams=disable txt.pdf txt.qdf 2>/dev/null
n=$(npages txt.pdf)
if [ "$n" = 1 ] && grep -aq '/FontFile2' txt.qdf; then
  ok "texttopdf: a 1-page PDF with an embedded TrueType font ($(grep -ao '/BaseFont */[A-Za-z+-]*' txt.qdf | head -n 1))"
else bad "texttopdf: $n pages, '$(head -c 20 txt.pdf | tr -d '\0')' $(grep -i 'error\|font' texttopdf.err | tail -n 3)"; fi

# --- imagetopdf ---
{ printf 'P6\n8 8\n255\n'; i=0; while [ $i -lt 64 ]; do printf '\377\000\000'; i=$((i+1)); done; } > red.ppm
CONTENT_TYPE=image/x-portable-pixmap filt imagetopdf "" red.ppm img.pdf
[ "$(npages img.pdf)" = 1 ] && ok "imagetopdf: PPM image to a 1-page PDF" || bad "imagetopdf: $(head -c 20 img.pdf | tr -d '\0') $(tail -n 3 imagetopdf.err)"

# --- driverless against ippeveprinter, then pdftoraster / rastertopdf ---
PORT=$((18631 + $$ % 300))
if command -v ippeveprinter >/dev/null 2>&1; then
  mkdir spool
  # ippeveprinter (built with Avahi) opens an Avahi client even with -r off,
  # and avahi_client_new() needs a D-Bus system bus (AVAHI_CLIENT_NO_FAIL
  # only covers a missing avahi-daemon): without one it exits "Unable to
  # initialize DNS-SD". A test target has no system bus, so give it a private
  # bus of its own (no avahi-daemon on it: nothing is announced).
  if [ -z "$DBUS_SYSTEM_BUS_ADDRESS" ] && [ ! -S /var/run/dbus/system_bus_socket ] && [ -x /usr/local/bin/dbus-daemon ]; then
    /usr/local/bin/dbus-daemon --config-file=/usr/local/share/dbus-1/session.conf --address="unix:path=$T/bus" \
      --nofork --nopidfile > dbus.log 2>&1 &
    DPID=$!
    i=0; while [ ! -S "$T/bus" ] && [ $i -lt 10 ]; do sleep 1; i=$((i+1)); done
    DBUS_SYSTEM_BUS_ADDRESS="unix:path=$T/bus"; export DBUS_SYSTEM_BUS_ADDRESS
    echo "  info private D-Bus for ippeveprinter's Avahi client: $DBUS_SYSTEM_BUS_ADDRESS"
  fi
  # -n localhost: ippeveprinter needs both an IPv4 and an IPv6 listener, and
  # CUPS maps "localhost" to the loopback address of each family.
  ippeveprinter -r off -n localhost -p $PORT -d "$T/spool" -M QNXTest -m "Raster 1" \
    -f image/pwg-raster "QNX Test Printer" > eve.log 2>&1 &
  EPID=$!
  i=0; while [ $i -lt 30 ]; do
    ipptool -q ipp://127.0.0.1:$PORT/ipp/print get-printer-attributes.test >/dev/null 2>&1 && break
    kill -0 $EPID 2>/dev/null || break; sleep 1; i=$((i+1))
  done
  driverless ipp://127.0.0.1:$PORT/ipp/print > p.ppd 2> driverless.err; rc=$?
  if [ $rc -eq 0 ] && grep -q '^\*PPD-Adobe' p.ppd; then
    ok "driverless: PPD generated from the IPP printer's attributes"
    grep -q '^\*Manufacturer: *"QNXTest"' p.ppd && grep -q '^\*ModelName: *"QNXTest' p.ppd \
      && ok "PPD: the printer's make, QNXTest" || bad "PPD make/model: $(grep -E '^\*(Manufacturer|ModelName)' p.ppd)"
    grep -q '^\*cupsFilter2: *"image/pwg-raster image/pwg-raster [0-9]* -"' p.ppd && ok "PPD: cupsFilter2 image/pwg-raster (the printer's only format)" || bad "PPD filters: $(grep cupsFilter p.ppd)"
    grep -q '^\*PageSize ' p.ppd && ok "PPD: page sizes" || bad "PPD: no *PageSize"
    PPD=$T/p.ppd; export PPD
    filt pdftoraster "" in.pdf out.ras
    s=$(head -c 4 out.ras)
    case "$s" in RaS2|RaS3|2SaR|3SaR) ok "pdftoraster: CUPS raster ($s)" ;; *) bad "pdftoraster: sync '$s' $(tail -n 3 pdftoraster.err)" ;; esac
    filt rastertopdf "" out.ras ras.pdf
    n=$(npages ras.pdf); [ "$n" = 2 ] && ok "rastertopdf: the raster back to a 2-page PDF" || bad "rastertopdf: $n pages $(tail -n 3 rastertopdf.err)"
    unset PPD
  else
    bad "driverless rc=$rc: $(head -n 3 driverless.err) / $(tail -n 3 eve.log)"
  fi
else bad "ippeveprinter (cups) not installed"; fi

[ $fail -eq 0 ] && { echo "PASS cups-filters"; exit 0; } || { echo "FAIL cups-filters ($fail check(s) failed)"; exit 1; }
