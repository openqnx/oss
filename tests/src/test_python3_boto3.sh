#!/bin/sh
# boto3 / botocore / s3transfer / jmespath / urllib3: the AWS SDK for Python
# against a small S3-compatible server on 127.0.0.1, started by the test in a
# thread (no network, no AWS account). Checks: create a bucket, put, head, get
# and delete objects (bytes and metadata round trip, ETag = MD5), SigV4
# Authorization header on every request, ListObjectsV2 with a paginator and a
# JMESPath search, a 404 surfacing as ClientError NoSuchKey, s3transfer's
# multipart upload/download of a 6 MiB file (3 parts) with an identical copy
# back, botocore's Stubber (no HTTP at all), jmespath and urllib3 on their own.
echo "== python3-boto3 =="
PY=${QNX_PORT_PYTHON:-/usr/local/bin/python3}; [ -x "$PY" ] || PY=python3
T=$(mktemp -d "${TMPDIR:-/tmp}/qnxport-boto3.XXXXXX") || { echo "FAIL python3-boto3 (no temporary directory)"; exit 1; }
cd "$T" || exit 1
cat > t.py <<'P'
import sys, os, io, hashlib, threading, re, datetime
from urllib.parse import urlsplit, parse_qs
from http.server import ThreadingHTTPServer, BaseHTTPRequestHandler
from xml.sax.saxutils import escape
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

# ---- a minimal S3: buckets, objects, ListObjectsV2, multipart uploads ----
store = {}          # bucket -> {key: (bytes, metadata dict, content-type)}
uploads = {}        # upload id -> {part number: bytes}
auth_seen = []
def dechunk(body):
    # aws-chunked (streaming SigV4 / trailer checksums): "<hex>[;ext]\r\n<data>\r\n" ... "0\r\n"
    out, i = b"", 0
    while True:
        j = body.index(b"\r\n", i); size = int(body[i:j].split(b";")[0], 16); i = j + 2
        if size == 0: return out
        out += body[i:i + size]; i += size + 2
class S3(BaseHTTPRequestHandler):
    protocol_version = "HTTP/1.1"
    def log_message(self, *a): pass
    def reply(self, code, body=b"", headers=None, ctype="application/xml"):
        self.send_response(code)
        for k, v in (headers or {}).items(): self.send_header(k, v)
        self.send_header("Content-Type", ctype); self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        if self.command != "HEAD": self.wfile.write(body)
    def error(self, code, ecode, msg):
        self.reply(code, ("<?xml version=\"1.0\"?><Error><Code>%s</Code><Message>%s</Message></Error>" % (ecode, msg)).encode())
    def parse(self):
        u = urlsplit(self.path); q = parse_qs(u.query, keep_blank_values=True)
        parts = u.path.lstrip("/").split("/", 1)
        auth_seen.append(self.headers.get("Authorization", ""))
        n = int(self.headers.get("Content-Length") or 0)
        body = self.rfile.read(n) if n else b""
        if "aws-chunked" in (self.headers.get("Content-Encoding") or "") or \
           (self.headers.get("x-amz-content-sha256") or "").startswith("STREAMING"):
            body = dechunk(body)
        from urllib.parse import unquote
        return parts[0], unquote(parts[1]) if len(parts) > 1 else "", q, body
    def do_PUT(self):
        b, k, q, body = self.parse()
        if not k:
            store.setdefault(b, {}); return self.reply(200, headers={"Location": "/" + b})
        if b not in store: return self.error(404, "NoSuchBucket", b)
        if "uploadId" in q:
            uploads[q["uploadId"][0]][int(q["partNumber"][0])] = body
            return self.reply(200, headers={"ETag": '"%s"' % hashlib.md5(body).hexdigest()})
        meta = {h[11:].lower(): v for h, v in self.headers.items() if h.lower().startswith("x-amz-meta-")}
        store[b][k] = (body, meta, self.headers.get("Content-Type", "binary/octet-stream"))
        self.reply(200, headers={"ETag": '"%s"' % hashlib.md5(body).hexdigest()})
    def do_POST(self):
        b, k, q, body = self.parse()
        if "uploads" in q:
            uid = "up%d" % len(uploads); uploads[uid] = {}
            return self.reply(200, ("<InitiateMultipartUploadResult><Bucket>%s</Bucket><Key>%s</Key><UploadId>%s</UploadId></InitiateMultipartUploadResult>" % (b, escape(k), uid)).encode())
        if "uploadId" in q:
            p = uploads.pop(q["uploadId"][0]); nums = [int(x) for x in re.findall(rb"<PartNumber>(\d+)</PartNumber>", body)]
            data = b"".join(p[i] for i in nums)
            store[b][k] = (data, {}, "binary/octet-stream")
            return self.reply(200, ("<CompleteMultipartUploadResult><Bucket>%s</Bucket><Key>%s</Key><ETag>\"%s-%d\"</ETag></CompleteMultipartUploadResult>" % (b, escape(k), hashlib.md5(data).hexdigest(), len(nums))).encode())
        self.error(400, "InvalidRequest", "unsupported POST")
    def obj(self):
        b, k, q, _ = self.parse()
        if b not in store: return self.error(404, "NoSuchBucket", b)
        if not k:   # ListObjectsV2 (list-type=2) or ListObjects (marker)
            pre = q.get("prefix", [""])[0]; mx = int(q.get("max-keys", ["1000"])[0])
            start = q.get("continuation-token", q.get("marker", [""]))[0]
            keys = sorted(x for x in store[b] if x.startswith(pre) and x > start)
            page, more = keys[:mx], len(keys) > mx
            xml = "<ListBucketResult><Name>%s</Name><Prefix>%s</Prefix><KeyCount>%d</KeyCount><MaxKeys>%d</MaxKeys><IsTruncated>%s</IsTruncated>" % (b, escape(pre), len(page), mx, "true" if more else "false")
            for x in page:
                xml += "<Contents><Key>%s</Key><LastModified>2026-01-02T03:04:05.000Z</LastModified><ETag>\"%s\"</ETag><Size>%d</Size><StorageClass>STANDARD</StorageClass></Contents>" % (escape(x), hashlib.md5(store[b][x][0]).hexdigest(), len(store[b][x][0]))
            if more: xml += ("<NextContinuationToken>%s</NextContinuationToken>" if q.get("list-type") == ["2"] else "<NextMarker>%s</NextMarker>") % escape(page[-1])
            return self.reply(200, (xml + "</ListBucketResult>").encode())
        if k not in store[b]: return self.error(404, "NoSuchKey", k)
        data, meta, ctype = store[b][k]
        h = {"ETag": '"%s"' % hashlib.md5(data).hexdigest(), "Last-Modified": "Fri, 02 Jan 2026 03:04:05 GMT"}
        for m, v in meta.items(): h["x-amz-meta-" + m] = v
        rng = self.headers.get("Range")
        if rng:
            a, z = rng.split("=")[1].split("-"); a = int(a); z = min(int(z) if z else len(data) - 1, len(data) - 1)
            h["Content-Range"] = "bytes %d-%d/%d" % (a, z, len(data))
            self.send_response(206)
            for kk, v in h.items(): self.send_header(kk, v)
            part = data[a:z + 1]
            self.send_header("Content-Type", ctype); self.send_header("Content-Length", str(len(part))); self.end_headers()
            return self.wfile.write(part)
        if self.command == "HEAD":
            self.send_response(200)
            for kk, v in h.items(): self.send_header(kk, v)
            self.send_header("Content-Type", ctype); self.send_header("Content-Length", str(len(data))); self.end_headers(); return
        self.reply(200, data, h, ctype)
    do_GET = obj
    do_HEAD = obj
    def do_DELETE(self):
        b, k, q, _ = self.parse()
        store.get(b, {}).pop(k, None); self.reply(204)

srv = ThreadingHTTPServer(("127.0.0.1", 0), S3)
threading.Thread(target=srv.serve_forever, daemon=True).start()
url = "http://127.0.0.1:%d" % srv.server_address[1]
info("stub S3 at " + url)

import boto3, botocore, s3transfer, jmespath, urllib3, dateutil
from botocore.config import Config
from botocore.exceptions import ClientError
info("boto3 %s, botocore %s, s3transfer %s, jmespath %s, urllib3 %s" % (boto3.__version__, botocore.__version__, s3transfer.__version__, jmespath.__version__, urllib3.__version__))
for m in (boto3, botocore, s3transfer, jmespath, urllib3):
    check(os.path.dirname(m.__file__).startswith(sys.prefix) or "site-packages" in m.__file__, "%s imported from site-packages" % m.__name__)
os.environ.pop("AWS_PROFILE", None)
sess = boto3.session.Session(aws_access_key_id="AKIDEXAMPLE", aws_secret_access_key="wJalrXUtnFEMI/K7MDENG+bPxRfiCYEXAMPLEKEY", region_name="us-east-1")
cfg = Config(s3={"addressing_style": "path"}, retries={"max_attempts": 1}, connect_timeout=5, read_timeout=20,
             request_checksum_calculation="when_required", response_checksum_validation="when_required")
s3 = sess.client("s3", endpoint_url=url, config=cfg)

def basic():
    s3.create_bucket(Bucket="qnx")
    payload = b"Hello from QNX\n" * 100
    r = s3.put_object(Bucket="qnx", Key="dir/hello.txt", Body=payload, ContentType="text/plain", Metadata={"origin": "qnx-port-test"})
    check(r["ETag"] == '"%s"' % hashlib.md5(payload).hexdigest(), "put_object: ETag is the MD5 of the body")
    h = s3.head_object(Bucket="qnx", Key="dir/hello.txt")
    check(h["ContentLength"] == len(payload) and h["Metadata"] == {"origin": "qnx-port-test"}, "head_object: length and user metadata")
    check(isinstance(h["LastModified"], datetime.datetime) and h["LastModified"].year == 2026 and h["LastModified"].tzinfo is not None, "LastModified parsed to an aware datetime (dateutil)")
    g = s3.get_object(Bucket="qnx", Key="dir/hello.txt")
    check(g["Body"].read() == payload and g["ContentType"] == "text/plain", "get_object returns the bytes and content type")
    g = s3.get_object(Bucket="qnx", Key="dir/hello.txt", Range="bytes=6-9")
    check(g["Body"].read() == b"from" and g["ResponseMetadata"]["HTTPStatusCode"] == 206, "ranged get_object: 206 with bytes 6-9")
    sig = [a for a in auth_seen if a]
    check(len(sig) == len(auth_seen) and all(a.startswith("AWS4-HMAC-SHA256 Credential=AKIDEXAMPLE/") and "/us-east-1/s3/aws4_request" in a and "Signature=" in a for a in sig), "every request signed with SigV4 (%d requests)" % len(sig))
    try:
        s3.get_object(Bucket="qnx", Key="missing"); check(False, "missing key raises")
    except ClientError as e:
        check(e.response["Error"]["Code"] == "NoSuchKey" and e.response["ResponseMetadata"]["HTTPStatusCode"] == 404, "missing key: ClientError NoSuchKey (404)")
def listing():
    for i in range(7):
        s3.put_object(Bucket="qnx", Key="list/item%02d" % i, Body=b"x" * (i + 1))
    pages = list(s3.get_paginator("list_objects_v2").paginate(Bucket="qnx", Prefix="list/", PaginationConfig={"PageSize": 3}))
    check(len(pages) == 3 and [p["KeyCount"] for p in pages] == [3, 3, 1], "paginator: 7 keys in pages of 3, 3, 1")
    it = s3.get_paginator("list_objects_v2").paginate(Bucket="qnx", Prefix="list/", PaginationConfig={"PageSize": 3})
    big = list(it.search("Contents[?Size > `4`].Key"))
    check(big == ["list/item04", "list/item05", "list/item06"], "paginator JMESPath search: keys with Size > 4")
    s3.delete_object(Bucket="qnx", Key="list/item00")
    r = s3.list_objects_v2(Bucket="qnx", Prefix="list/")
    check(r["KeyCount"] == 6 and "list/item00" not in [c["Key"] for c in r["Contents"]], "delete_object removes the key")
def transfer():
    from boto3.s3.transfer import TransferConfig
    data = os.urandom(6 * 1024 * 1024)
    open("big.bin", "wb").write(data)
    tc = TransferConfig(multipart_threshold=5 * 1024 * 1024, multipart_chunksize=2 * 1024 * 1024, max_concurrency=2)
    s3.upload_file("big.bin", "qnx", "big.bin", Config=tc)
    stored = store["qnx"]["big.bin"][0]
    check(stored == data, "upload_file: 6 MiB multipart upload stored intact")
    check(not uploads, "multipart upload completed (no upload left open)")
    s3.download_file("qnx", "big.bin", "back.bin", Config=tc)
    check(open("back.bin", "rb").read() == data, "download_file (ranged, concurrent) gives an identical copy")
    res = sess.resource("s3", endpoint_url=url, config=cfg)
    keys = sorted(o.key for o in res.Bucket("qnx").objects.filter(Prefix="big"))
    check(keys == ["big.bin"], "resource API: Bucket.objects.filter")
def stubber():
    from botocore.stub import Stubber
    c = sess.client("s3", endpoint_url="http://127.0.0.1:9", config=cfg)
    with Stubber(c) as st:
        st.add_response("list_buckets", {"Buckets": [{"Name": "stubbed", "CreationDate": datetime.datetime(2026, 1, 1)}], "Owner": {"ID": "x"}})
        st.add_client_error("head_bucket", service_error_code="404", http_status_code=404)
        check(c.list_buckets()["Buckets"][0]["Name"] == "stubbed", "Stubber: canned list_buckets response")
        try:
            c.head_bucket(Bucket="nope"); check(False, "stubbed error raised")
        except ClientError as e:
            check(e.response["ResponseMetadata"]["HTTPStatusCode"] == 404, "Stubber: canned ClientError")
        st.assert_no_pending_responses()
    check(True, "Stubber: every queued response consumed")
    ddb = sess.client("dynamodb", endpoint_url="http://127.0.0.1:9")
    check("put_item" in dir(ddb) and ddb.meta.service_model.service_name == "dynamodb", "service models load (dynamodb client built from botocore data)")
def jp():
    d = {"a": [{"n": 1, "t": "x"}, {"n": 5, "t": "y"}, {"n": 3, "t": "x"}]}
    check(jmespath.search("a[?t=='x'].n", d) == [1, 3], "jmespath filter")
    check(jmespath.search("max_by(a, &n).t", d) == "y" and jmespath.search("length(a)", d) == 3, "jmespath functions")
def u3():
    store.setdefault("raw", {})["k"] = (b"urllib3 body", {}, "text/plain")
    pm = urllib3.PoolManager()
    r = pm.request("GET", url + "/raw/k"); r2 = pm.request("GET", url + "/raw/k")
    check(r.status == 200 and r.data == b"urllib3 body" and r2.data == r.data, "urllib3 PoolManager GET (connection reused)")
    check(pm.request("GET", url + "/raw/none").status == 404, "urllib3: 404 status")
for n, fn in (("basic", basic), ("listing", listing), ("transfer", transfer), ("stubber", stubber), ("jmespath", jp), ("urllib3", u3)):
    run(n, fn)
srv.shutdown()
sys.exit(1 if fails else 0)
P
"$PY" t.py; rc=$?
cd /; rm -rf "$T"
[ $rc -eq 0 ] && { echo "PASS python3-boto3"; exit 0; }
echo "FAIL python3-boto3 (rc=$rc)"; exit 1
