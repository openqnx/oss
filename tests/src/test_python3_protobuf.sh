#!/bin/sh
# python3-protobuf: the upb C extension is the implementation in use; a
# message type built at run time from a FileDescriptorProto serialises to
# exactly the bytes the wire format prescribes (varint 150 = 96 01, a
# length-delimited string, packed doubles, a bool; -1 as a ten-byte varint),
# parses back, and converts to and from JSON and text format; the
# Timestamp well-known type formats RFC 3339. Then the ported protoc (29.3)
# generates Python code for a .proto on the target, and this runtime
# (5.29.3, its Python release) imports it and gives the same bytes.
echo "== python3-protobuf =="
PY=${QNX_PORT_PYTHON:-/usr/local/bin/python3}; [ -x "$PY" ] || PY=python3
BIN=$(dirname "$PY")
# No __pycache__ left in the installation's site-packages (docs/porting-hazards.md 183)
export PYTHONDONTWRITEBYTECODE=1
T=$(mktemp -d "${TMPDIR:-/tmp}/qnxport-pyprotobuf.XXXXXX") || { echo "FAIL python3-protobuf (no temporary directory)"; exit 1; }
trap 'cd /; rm -rf "$T"' EXIT
cd "$T" || exit 1
PROTOC="$BIN/protoc"; [ -x "$PROTOC" ] || PROTOC=protoc
cat > qnx.proto <<'P'
syntax = "proto3";
package qnx;
message Reading {
  int32 id = 1;
  string name = 2;
  repeated double values = 3;
  bool flag = 4;
}
P
"$PROTOC" --python_out=. qnx.proto > protoc.out 2>&1 || { echo "  info protoc: $(cat protoc.out)"; }
cat > t.py <<'P'
import json, os, sys
fails = 0
def check(ok, what):
    global fails
    print(("  ok   " if ok else "  FAIL ") + what); fails += (not ok)
def info(s): print("  info " + s)
import google.protobuf
from google.protobuf import descriptor_pb2, descriptor_pool, message_factory, json_format, text_format, timestamp_pb2
from google.protobuf.internal import api_implementation
info("protobuf %s, implementation %s" % (google.protobuf.__version__, api_implementation.Type()))
check(google.protobuf.__version__ == "5.29.3", "runtime version 5.29.3 (protobuf 29.3)")
check(api_implementation.Type() == "upb", "the upb C extension is in use")
F = descriptor_pb2.FieldDescriptorProto
fdp = descriptor_pb2.FileDescriptorProto(name="dyn.proto", package="dyn", syntax="proto3")
m = fdp.message_type.add(name="Reading")
for n, num, t, lab in (("id", 1, F.TYPE_INT32, F.LABEL_OPTIONAL), ("name", 2, F.TYPE_STRING, F.LABEL_OPTIONAL),
                       ("values", 3, F.TYPE_DOUBLE, F.LABEL_REPEATED), ("flag", 4, F.TYPE_BOOL, F.LABEL_OPTIONAL)):
    m.field.add(name=n, number=num, type=t, label=lab)
pool = descriptor_pool.DescriptorPool(); pool.Add(fdp)
Reading = message_factory.GetMessageClass(pool.FindMessageTypeByName("dyn.Reading"))
r = Reading(id=150, name="qnx", values=[1.0, 2.5], flag=True)
want = bytes.fromhex("089601120371" "6e781a10000000000000f03f00000000000004402001")
b = r.SerializeToString()
check(b == want, "serialised bytes are exactly the wire format")
if b != want: info(b.hex())
r2 = Reading(); r2.ParseFromString(b)
check(r2 == r and r2.id == 150 and list(r2.values) == [1.0, 2.5], "parses back to an equal message")
check(Reading(id=-1).SerializeToString() == bytes.fromhex("08ffffffffffffffffff01"), "int32 -1 is a ten-byte varint")
j = json.loads(json_format.MessageToJson(r))
check(j == {"id": 150, "name": "qnx", "values": [1.0, 2.5], "flag": True}, "MessageToJson")
check(json_format.Parse('{"id": 7, "values": [3]}', Reading()) == Reading(id=7, values=[3.0]), "json_format.Parse")
check(text_format.MessageToString(Reading(id=3, name="a")) == 'id: 3\nname: "a"\n', "text format")
# Timestamp.FromJsonString() is not used: it parses with datetime.strptime(),
# which fails in the python3 port when no LC_ALL/LANG is set (QNX's
# setlocale(LC_TIME, NULL) answers "C;ctype:POSIX", which locale.getlocale()
# cannot parse) -- a python3 defect, not protobuf's.
ts = timestamp_pb2.Timestamp(seconds=1, nanos=500000000)
check(ts.ToJsonString() == "1970-01-01T00:00:01.500Z" and ts.ToMilliseconds() == 1500, "Timestamp to RFC 3339")
if os.path.exists("qnx_pb2.py"):
    sys.path.insert(0, os.getcwd())
    import qnx_pb2
    g = qnx_pb2.Reading(id=150, name="qnx", values=[1.0, 2.5], flag=True)
    check(g.SerializeToString() == want, "protoc-generated qnx_pb2 imports and gives the same bytes")
else:
    check(False, "protoc --python_out generated qnx_pb2.py")
sys.exit(1 if fails else 0)
P
"$PY" t.py; rc=$?
[ $rc -eq 0 ] && { echo "PASS python3-protobuf"; exit 0; }
echo "FAIL python3-protobuf (rc=$rc)"; exit 1
