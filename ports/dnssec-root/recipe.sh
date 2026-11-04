# The DNSSEC root zone trust anchors, from IANA's root-anchors.xml: the
# root KSKs (20326, and 38696 of the 2024 rollover) as DNSKEY records in
# share/dnssec-root/trusted-key.key -- the file validating resolvers
# (unbound's trust-anchor-file, dnsmasq, ldns tools) are pointed at -- and
# the XML itself next to it (unbound-anchor's format). Data, the same for
# every architecture. VERSION is the date IANA published this content
# (Last-Modified of root-anchors.xml); the download is pinned by SHA-256.
VERSION=20241105
TARBALL=https://data.iana.org/root-anchors/root-anchors.xml
SHA256=3ccaab38830025ee0a0f6c1f25769427544f81ea2865aa860468f3ef5278b908
BUILDSYS=custom
do_fetch() { mkdir -p "$SRCROOT" "$WS/dist" && local f="$WS/dist/dnssec-root-anchors-$VERSION.xml"; [ -f "$f" ] || curl -fsSL -o "$f" "$TARBALL"; \
  echo "$SHA256  $f" | sha256sum -c --status && cp "$f" "$SRCROOT/root-anchors.xml"; }
# trusted-key.key: one ". IN DNSKEY <flags> 3 <alg> <key>" line per anchor
# that is valid now (has no validUntil) and carries its public key, as the
# Alpine/QNX packages generate it with IANA's get_trust_anchor.py.
do_build() {
  mkdir -p "$B" && python3 -I - "$S/root-anchors.xml" "$B/trusted-key.key" <<'PY'
import sys, xml.etree.ElementTree as ET
t = ET.parse(sys.argv[1]).getroot()
lines = []
for kd in t.findall("KeyDigest"):
    pk = kd.findtext("PublicKey")
    if kd.get("validUntil") or not pk:
        continue
    lines.append(". IN DNSKEY %s 3 %s %s" % (kd.findtext("Flags"), kd.findtext("Algorithm"), pk.strip()))
if not lines:
    sys.exit("no current trust anchor with a public key")
open(sys.argv[2], "w").write("\n".join(sorted(lines, key=lambda l: l.split()[6])) + "\n")
PY
}
do_install() { install -Dm644 "$B/trusted-key.key" "$STAGE$PREFIX/share/dnssec-root/trusted-key.key" &&
  install -Dm644 "$S/root-anchors.xml" "$STAGE$PREFIX/share/dnssec-root/root-anchors.xml"; }
