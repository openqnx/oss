#!/bin/sh
# python3-msgpack and python3-zstandard, both with their C extensions:
# msgpack packs values to the exact bytes of the MessagePack spec, round
# trips nested data (ints of every width, floats, bytes, str, lists, maps,
# ext types) and streams through Unpacker; zstandard compresses with the C
# backend on the system libzstd -- the frame starts with the zstd magic,
# records the content size, round trips one-shot and streaming, at several
# levels, and with a trained dictionary.
echo "== python3-msgpack-zstandard =="
PY=${QNX_PORT_PYTHON:-/usr/local/bin/python3}; [ -x "$PY" ] || PY=python3
# No __pycache__ left in the installation's site-packages (docs/porting-hazards.md 183)
export PYTHONDONTWRITEBYTECODE=1
T=$(mktemp -d "${TMPDIR:-/tmp}/qnxport-msgzst.XXXXXX") || { echo "FAIL python3-msgpack-zstandard (no temporary directory)"; exit 1; }
trap 'cd /; rm -rf "$T"' EXIT
cd "$T" || exit 1
cat > t.py <<'P'
import sys, io, os, random
fails = 0
def check(ok, what):
    global fails
    print(("  ok   " if ok else "  FAIL ") + what); fails += (not ok)
def info(s): print("  info " + s)
import msgpack, msgpack._cmsgpack as cm
info("msgpack %s, extension %s" % (".".join(map(str, msgpack.version)), cm.__file__))
check(msgpack.Packer is cm.Packer and msgpack.Unpacker is cm.Unpacker, "msgpack uses the C extension (_cmsgpack)")
check(msgpack.packb({"a": 1}) == b"\x81\xa1a\x01", "packb({'a': 1}) == 81 a1 61 01")
check(msgpack.packb([1, -1, 300, 2**40, None, True]) == b"\x96\x01\xff\xcd\x01\x2c\xcf\x00\x00\x01\x00\x00\x00\x00\x00\xc0\xc3", "fixint, negative fixint, uint16, uint64, nil, true")
check(msgpack.packb(1.5) == b"\xcb\x3f\xf8" + b"\x00" * 6 and msgpack.packb(b"\x00\x01") == b"\xc4\x02\x00\x01", "float64 and bin8 encodings")
data = {"name": "qnx", "list": list(range(-40, 300, 7)), "nested": {"f": 3.25, "b": b"\xff" * 300, "u": "ü中"}, "big": -2**63, "ubig": 2**64 - 1}
check(msgpack.unpackb(msgpack.packb(data)) == data, "nested data round trips")
ext = msgpack.ExtType(5, b"abc")
check(msgpack.unpackb(msgpack.packb(ext)) == ext, "ext type round trips")
buf = io.BytesIO(b"".join(msgpack.packb(i) for i in range(1000)))
u = msgpack.Unpacker(buf)
check(list(u) == list(range(1000)), "Unpacker streams 1000 objects")
try:
    msgpack.unpackb(b"\x92\x01"); check(False, "truncated input raises")
except (ValueError, msgpack.exceptions.UnpackValueError):
    check(True, "truncated input raises ValueError")
import zstandard as zstd
info("zstandard %s, backend %s, libzstd %s" % (zstd.__version__, zstd.backend, ".".join(map(str, zstd.ZSTD_VERSION))))
check(zstd.backend == "cext", "zstandard uses the C extension backend")
plain = (b"QNX Neutrino RTOS " * 2000) + os.urandom(1000)
c = zstd.ZstdCompressor(level=3).compress(plain)
check(c[:4] == b"\x28\xb5\x2f\xfd", "frame starts with the zstd magic 28 b5 2f fd")
fp = zstd.get_frame_parameters(c)
check(fp.content_size == len(plain), "frame records the content size")
check(len(c) < len(plain) // 10 and zstd.ZstdDecompressor().decompress(c) == plain, "one-shot round trip, compressed below 10%")
for lvl in (1, 9, 19):
    check(zstd.ZstdDecompressor().decompress(zstd.ZstdCompressor(level=lvl).compress(plain)) == plain, "round trip at level %d" % lvl)
out = io.BytesIO()
with zstd.ZstdCompressor(level=5).stream_writer(out, closefd=False) as w:
    for i in range(0, len(plain), 4096): w.write(plain[i:i + 4096])
back = zstd.ZstdDecompressor().stream_reader(io.BytesIO(out.getvalue())).read()
check(back == plain, "streaming compress and decompress")
random.seed(1)
samples = [("{\"id\": %d, \"host\": \"node%d\", \"state\": \"%s\"}" % (i, i % 7, random.choice(["up", "down", "idle"]))).encode() for i in range(500)]
d = zstd.train_dictionary(2048, samples)
cc = zstd.ZstdCompressor(dict_data=d); dd = zstd.ZstdDecompressor(dict_data=d)
check(all(dd.decompress(cc.compress(s)) == s for s in samples[:50]), "dictionary trained, round trips with it")
try:
    zstd.ZstdDecompressor().decompress(b"\x28\xb5\x2f\xfd" + b"\x00" * 10); check(False, "corrupt frame raises ZstdError")
except zstd.ZstdError:
    check(True, "corrupt frame raises ZstdError")
sys.exit(1 if fails else 0)
P
"$PY" t.py; rc=$?
[ $rc -eq 0 ] && { echo "PASS python3-msgpack-zstandard"; exit 0; }
echo "FAIL python3-msgpack-zstandard (rc=$rc)"; exit 1
