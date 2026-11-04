#!/bin/sh
# colcon-common-extensions and what it pulls in: colcon on a workspace of
# three packages -- a ROS package (package.xml, catkin_pkg), a plain CMake
# package with a colcon.pkg dependency (colcon-metadata), a setup.py Python
# package -- with known answers from `colcon list`, `graph`, `info` and
# `extensions` (one extension of every common-extensions package is
# registered), then `colcon build` of the Python package and the generated
# setup scripts (empy templates). Also each library underneath directly:
# argcomplete, PyYAML, docutils, pyparsing, empy, distlib, catkin_pkg, the
# pytest plugins.
echo "== python3-colcon-common-extensions =="
PY=${QNX_PORT_PYTHON:-/usr/local/bin/python3}; [ -x "$PY" ] || PY=python3
COLCON=/usr/local/bin/colcon; [ -x "$COLCON" ] || COLCON=colcon
T=$(mktemp -d "${TMPDIR:-/tmp}/qnxport-colcon.XXXXXX") || { echo "FAIL python3-colcon-common-extensions (no temporary directory)"; exit 1; }
cd "$T" || exit 1
fails=0
ok() { echo "  ok   $1"; }
bad() { echo "  FAIL $1"; fails=$((fails + 1)); }
chk() { if [ "$1" = 0 ]; then ok "$2"; else bad "$2"; fi; }
export HOME="$T" COLCON_HOME="$T/.colcon" COLCON_LOG_PATH="$T/log"
unset COLCON_PREFIX_PATH AMENT_PREFIX_PATH CMAKE_PREFIX_PATH

# ---- the libraries, directly ----
cat > libs.py <<'P'
import sys, argparse
fails = 0
def check(ok, what):
    global fails
    print(("  ok   " if ok else "  FAIL ") + what); fails += (not ok)
def run(name, fn):
    try: fn()
    except Exception as e:
        import traceback; traceback.print_exc(limit=3, file=sys.stdout)
        check(False, "%s: %r" % (name, e))
def t_argcomplete():
    import argcomplete
    p = argparse.ArgumentParser(prog="qnxprog")
    p.add_argument("--format", choices=["json", "yaml"]); p.add_argument("--force", action="store_true")
    f = argcomplete.CompletionFinder(p)
    check(sorted(f._get_completions(["qnxprog"], "--fo", "", None)) == ["--force", "--format"], "argcomplete: --fo completes to --force, --format")
    check([c.strip() for c in f._get_completions(["qnxprog", "--format"], "y", "", None)] == ["yaml"], "argcomplete: --format y completes to yaml")
def t_yaml():
    import yaml
    d = yaml.safe_load("a: 1\nb: [x, y]\nc: {d: 2.5, e: null}\n")
    check(d == {"a": 1, "b": ["x", "y"], "c": {"d": 2.5, "e": None}}, "PyYAML: safe_load of a mapping")
    check(yaml.safe_dump({"k": [1, 2], "s": "q"}, sort_keys=True) == "k:\n- 1\n- 2\ns: q\n", "PyYAML: safe_dump")
def t_docutils():
    import docutils.core
    parts = docutils.core.publish_parts("Title\n=====\n\n*emph* and ``lit``\n", writer_name="html")
    check(parts["title"] == "Title" and "<em>emph</em>" in parts["body"], "docutils: reStructuredText to HTML")
def t_pyparsing():
    import pyparsing as pp
    integer = pp.Word(pp.nums).set_parse_action(lambda t: int(t[0]))
    expr = pp.infix_notation(integer, [(pp.one_of("* /"), 2, pp.OpAssoc.LEFT), (pp.one_of("+ -"), 2, pp.OpAssoc.LEFT)])
    check(expr.parse_string("2+3*4", parse_all=True).as_list() == [[2, "+", [3, "*", 4]]], "pyparsing: 2+3*4 groups as 2+(3*4)")
def t_empy():
    import em
    check(em.expand("@(1+2) @[if x]yes@[else]no@[end if]", {"x": True}) == "3 yes", "empy: expression and conditional")
def t_distlib():
    from distlib.version import NormalizedVersion as V
    import distlib.markers
    check(V("1.0.post1") > V("1.0") > V("1.0rc1") and distlib.markers.interpret("python_version >= '3.0'"), "distlib: version order, markers")
def t_catkin():
    from catkin_pkg.package import parse_package_string
    pk = parse_package_string(open("ws/src/qnx_msgs/package.xml").read())
    pk.evaluate_conditions({"ROS_VERSION": "2"})
    check(pk.name == "qnx_msgs" and pk.version == "1.2.3", "catkin_pkg: name and version from package.xml")
    check([d.name for d in pk.build_depends if d.evaluated_condition] == ["std_msgs"] and pk.licenses == ["Apache-2.0"], "catkin_pkg: conditional dependency, license")
def t_plugins():
    import pytest_repeat, pytest_rerunfailures, pytest_cov  # noqa: F401
    check(True, "pytest plugins import: repeat, rerunfailures, cov")
for n, f in (("argcomplete", t_argcomplete), ("yaml", t_yaml), ("docutils", t_docutils), ("pyparsing", t_pyparsing),
             ("empy", t_empy), ("distlib", t_distlib), ("catkin_pkg", t_catkin), ("plugins", t_plugins)):
    run(n, f)
sys.exit(1 if fails else 0)
P

# ---- a workspace ----
mkdir -p ws/src/qnx_msgs ws/src/qnx_core ws/src/qnx_tools
cat > ws/src/qnx_msgs/package.xml <<'E'
<?xml version="1.0"?>
<package format="3">
  <name>qnx_msgs</name>
  <version>1.2.3</version>
  <description>Messages</description>
  <maintainer email="m@example.com">M</maintainer>
  <license>Apache-2.0</license>
  <buildtool_depend>ament_cmake</buildtool_depend>
  <depend condition="$ROS_VERSION == 2">std_msgs</depend>
  <export><build_type>ament_cmake</build_type></export>
</package>
E
printf 'cmake_minimum_required(VERSION 3.5)\nproject(qnx_msgs)\n' > ws/src/qnx_msgs/CMakeLists.txt
printf 'cmake_minimum_required(VERSION 3.5)\nproject(qnx_core VERSION 0.4.0)\n' > ws/src/qnx_core/CMakeLists.txt
echo '{"dependencies": ["qnx_msgs"]}' > ws/src/qnx_core/colcon.pkg
cat > ws/src/qnx_tools/setup.py <<'E'
from setuptools import setup
setup(name='qnx_tools', version='2.0.1', py_modules=['qnx_tools'])
E
echo 'ANSWER = 42' > ws/src/qnx_tools/qnx_tools.py

"$PY" libs.py; chk $? "libraries underneath colcon"

cd ws || exit 1
"$COLCON" list > list.txt 2> list.err; rc=$?
sed 's/^/  info /' list.txt
chk $rc "colcon list runs"
grep -q '^qnx_msgs[[:space:]].*(ros.ament_cmake)' list.txt; chk $? "qnx_msgs identified as ros.ament_cmake (colcon-ros, catkin_pkg)"
grep -q '^qnx_core[[:space:]].*(cmake)' list.txt; chk $? "qnx_core identified as cmake (colcon-cmake)"
grep -q '^qnx_tools[[:space:]].*(python)' list.txt; chk $? "qnx_tools identified as python (colcon-python-setup-py)"
[ "$(wc -l < list.txt | tr -d ' ')" = 3 ]; chk $? "three packages found (colcon-recursive-crawl)"

"$COLCON" graph > graph.txt 2>&1
sed 's/^/  info /' graph.txt
m=$(awk '$1 == "qnx_msgs" { print NR }' graph.txt); c=$(awk '$1 == "qnx_core" { print NR }' graph.txt)
[ -n "$m" ] && [ -n "$c" ] && [ "$m" -lt "$c" ] && [ "$(wc -l < graph.txt | tr -d ' ')" = 3 ]
chk $? "graph: qnx_msgs ordered before qnx_core (colcon.pkg dependency, colcon-metadata)"
"$COLCON" list -t --packages-up-to qnx_core --names-only > upto.txt 2>&1
[ "$(tr '\n' ' ' < upto.txt)" = "qnx_msgs qnx_core " ]; chk $? "--packages-up-to qnx_core selects qnx_msgs, qnx_core (colcon-package-selection)"
"$COLCON" info qnx_msgs > info.txt 2>&1
grep -q 'version: 1.2.3' info.txt && grep -q 'build: ament_cmake' info.txt; chk $? "colcon info: version and build dependency (colcon-package-information)"

"$COLCON" extensions > ext.txt 2>&1; rc=$?
chk $rc "colcon extensions runs (colcon-devtools)"
for e in "argcomplete_completer:cmake_args:colcon-argcomplete" "shell:bash:colcon-bash" "shell:zsh:colcon-zsh" \
         "shell:powershell:colcon-powershell" "task.build:cmake:colcon-cmake" "package_identification:ros:colcon-ros" \
         "package_identification:python_setup_py:colcon-python-setup-py" "package_discovery:recursive:colcon-recursive-crawl" \
         "argument_parser:colcon:colcon-defaults" "package_augmentation:colcon_meta:colcon-metadata" \
         "event_handler:terminal_title:colcon-notification" "event_handler:log:colcon-output" \
         "package_selection:dependencies:colcon-package-selection" "executor:parallel:colcon-parallel-executor" \
         "environment:library_path:colcon-library-path" "environment:pkg_config:colcon-pkg-config"; do
  grp=${e%%:*}; rest=${e#*:}; name=${rest%%:*}; pkg=${rest#*:}
  awk -v g="colcon_core.$grp" -v g2="colcon_argcomplete.$grp" -v n="$name" '
    /^[^ ]/ { sec = $1 } $1 == n":" && (sec == g || sec == g2) { f = 1 } END { exit !f }' ext.txt
  chk $? "extension $grp $name registered ($pkg)"
done
"$COLCON" test-result --help > /dev/null 2>&1; chk $? "colcon test-result verb (colcon-test-result)"
cdfn=$("$PY" -c 'import sys; print(sys.prefix)')/share/colcon_cd/function/colcon_cd.sh
grep -q 'colcon_cd()' "$cdfn" 2>/dev/null; chk $? "colcon-cd shell function installed ($cdfn)"

"$COLCON" build --packages-select qnx_tools --event-handlers console_direct- > build.txt 2>&1; rc=$?
sed 's/^/  info /' build.txt | tail -5
chk $rc "colcon build qnx_tools (setup.py package)"
sp=$(ls -d install/qnx_tools/lib/python*/site-packages 2>/dev/null | head -1)
[ -n "$sp" ] && [ -f "$sp/qnx_tools.py" ]; chk $? "qnx_tools installed into install/qnx_tools"
[ "$(PYTHONPATH=$sp "$PY" -c 'import qnx_tools; print(qnx_tools.ANSWER)' 2>&1)" = 42 ]; chk $? "installed module imports: ANSWER = 42"
[ -f install/setup.sh ] && [ -f install/qnx_tools/share/qnx_tools/package.sh ] && [ -f install/setup.bash ]; chk $? "setup scripts generated (empy templates, colcon-bash)"
out=$(. install/setup.sh >/dev/null 2>&1; "$PY" -c 'import qnx_tools; print(qnx_tools.ANSWER)' 2>&1)
[ "$out" = 42 ]; chk $? "after sourcing install/setup.sh the package imports"

cd /; rm -rf "$T"
[ $fails -eq 0 ] && { echo "PASS python3-colcon-common-extensions"; exit 0; }
echo "FAIL python3-colcon-common-extensions ($fails check(s) failed)"; exit 1
