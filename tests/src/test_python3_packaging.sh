#!/bin/sh
# python3-packaging: PEP 440 versions (ordering, pre/post/dev releases,
# local versions), specifier sets, PEP 508 requirements with markers
# evaluated for this interpreter, wheel tags for this platform, and name and
# wheel-filename utilities.
echo "== python3-packaging =="
PY=${QNX_PORT_PYTHON:-/usr/local/bin/python3}; [ -x "$PY" ] || PY=python3
T=$(mktemp -d "${TMPDIR:-/tmp}/qnxport-packaging.XXXXXX") || { echo "FAIL python3-packaging (no temporary directory)"; exit 1; }
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
        import traceback; traceback.print_exc(limit=3, file=sys.stdout)
        check(False, "%s: %r" % (name, e))
import packaging
from packaging.version import Version, InvalidVersion
from packaging.specifiers import SpecifierSet
from packaging.requirements import Requirement
from packaging.markers import Marker, default_environment
from packaging import tags, utils
info("packaging %s" % packaging.__version__)
def versions():
    check(Version("1.10.0") > Version("1.9.9") and Version("2.0") == Version("2.0.0"), "1.10.0 > 1.9.9; 2.0 == 2.0.0")
    order = ["1.0.dev1", "1.0a1", "1.0b2", "1.0rc1", "1.0", "1.0.post1", "1.1"]
    check(sorted(order, key=Version) == order, "dev < alpha < beta < rc < final < post")
    v = Version("2.4.1rc2+qnx.8")
    check(v.is_prerelease and v.local == "qnx.8" and v.release == (2, 4, 1) and v.public == "2.4.1rc2", "2.4.1rc2+qnx.8: pre-release, local qnx.8")
    try: Version("not.a.version!"); check(False, "invalid version rejected")
    except InvalidVersion: check(True, "invalid version rejected")
def specs():
    s = SpecifierSet(">=1.0,<2.0,!=1.5.*")
    check("1.4" in s and "1.5.3" not in s and "2.0" not in s, ">=1.0,<2.0,!=1.5.*")
    check(list(s.filter(["0.9", "1.2", "1.5.1", "1.9", "2.1"])) == ["1.2", "1.9"], "filter a version list")
    check(Version("1.4.2") in SpecifierSet("~=1.4.0") and Version("1.5") not in SpecifierSet("~=1.4.0"), "compatible release ~=1.4.0")
    check("2.0rc1" not in SpecifierSet(">=1.0") and SpecifierSet(">=1.0").contains("2.0rc1", prereleases=True), "pre-releases excluded unless asked")
def reqs():
    r = Requirement('requests[security,socks]>=2.8.1,<3; python_version >= "3.8"')
    check(r.name == "requests" and r.extras == {"security", "socks"} and "2.31" in r.specifier, "requirement: name, extras, specifier")
    check(r.marker.evaluate() is True, "marker python_version >= 3.8 true here")
    env = default_environment()
    info("marker environment: platform_system=%s sys_platform=%s machine=%s" % (env["platform_system"], env["sys_platform"], env["platform_machine"]))
    check(Marker('sys_platform == "%s"' % sys.platform).evaluate() and not Marker('os_name == "nt"').evaluate(), "markers for this platform")
def tagstest():
    t = list(tags.sys_tags())
    info("%d supported tags, first %s" % (len(t), t[0] if t else None))
    check(len(t) > 10 and tags.Tag("py3", "none", "any") in t, "sys_tags includes py3-none-any")
    check(str(t[0]).startswith("cp%d%d" % sys.version_info[:2]), "first tag is for this CPython")
def util():
    check(utils.canonicalize_name("Foo_Bar.baz") == "foo-bar-baz", "canonicalize_name")
    name, ver, build, tg = utils.parse_wheel_filename("qnx_demo-1.2.3-py3-none-any.whl")
    check(name == "qnx-demo" and ver == Version("1.2.3") and tags.Tag("py3", "none", "any") in tg, "parse_wheel_filename")
    check(utils.parse_sdist_filename("qnx-demo-1.0.tar.gz")[1] == Version("1.0"), "parse_sdist_filename")
for n, f in (("version", versions), ("specifiers", specs), ("requirements", reqs), ("tags", tagstest), ("utils", util)):
    run(n, f)
sys.exit(1 if fails else 0)
P
"$PY" t.py; rc=$?
cd /; rm -rf "$T"
[ $rc -eq 0 ] && { echo "PASS python3-packaging"; exit 0; }
echo "FAIL python3-packaging (rc=$rc)"; exit 1
