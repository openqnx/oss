#!/bin/sh
# The ROS tool modules of group g16: vcstool (vcs import of a .repos file
# that names a local git repository, vcs status, vcs export --exact gives
# the commit), rosdistro (a ROS index and distribution file read from
# file:// URLs: repositories, release version, packages), rospkg (RosPack
# finds packages under a path and reads their package.xml).
echo "== python3-ros-tools =="
PY=${QNX_PORT_PYTHON:-/usr/local/bin/python3}; [ -x "$PY" ] || PY=python3
BIN=$(dirname "$PY")
# No __pycache__ left in the installation's site-packages (docs/porting-hazards.md 183)
export PYTHONDONTWRITEBYTECODE=1
T=$(mktemp -d "${TMPDIR:-/tmp}/qnxport-rostools.XXXXXX") || { echo "FAIL python3-ros-tools (no temporary directory)"; exit 1; }
trap 'cd /; rm -rf "$T"' EXIT
cd "$T" || exit 1
fails=0
ok()  { echo "  ok   $1"; }
bad() { echo "  FAIL $1"; fails=$((fails+1)); }
export HOME="$T" GIT_AUTHOR_NAME=qnx GIT_AUTHOR_EMAIL=qnx@example.invalid GIT_COMMITTER_NAME=qnx GIT_COMMITTER_EMAIL=qnx@example.invalid
VCS="$BIN/vcs"; [ -x "$VCS" ] || VCS=vcs
# ---- vcstool
mkdir -p upstream && ( cd upstream && git init -q -b main . && echo one > README && git add README && git commit -q -m first ) \
  || bad "git repository for vcstool"
REV=$(git -C upstream rev-parse HEAD)
printf 'repositories:\n  src/demo:\n    type: git\n    url: file://%s/upstream\n    version: main\n' "$T" > ws.repos
mkdir ws && "$VCS" import --input ws.repos ws > imp.out 2>&1; rc=$?
[ $rc -eq 0 ] && [ "$(cat ws/src/demo/README 2>/dev/null)" = one ] && ok "vcs import clones the repository" \
  || { bad "vcs import clones the repository (rc=$rc)"; sed 's/^/  info /' imp.out; }
"$VCS" export --exact ws > exp.out 2>&1
grep -q "version: $REV" exp.out && grep -q "src/demo:" exp.out && ok "vcs export --exact gives the commit" \
  || { bad "vcs export --exact gives the commit"; sed 's/^/  info /' exp.out; }
echo two >> ws/src/demo/README
"$VCS" status ws > st.out 2>&1
grep -q "modified: *README" st.out && ok "vcs status shows the modified file" || { bad "vcs status shows the modified file"; sed 's/^/  info /' st.out; }
# ---- rosdistro and rospkg
mkdir -p rd/jazzy pkgs/demo_a pkgs/sub/demo_b
cat > rd/index.yaml <<'Y'
%YAML 1.1
---
distributions:
  jazzy:
    distribution: [jazzy/distribution.yaml]
    distribution_status: active
    distribution_type: ros2
    python_version: 3
type: index
version: 4
Y
cat > rd/jazzy/distribution.yaml <<'Y'
%YAML 1.1
---
release_platforms:
  ubuntu: [noble]
repositories:
  demo:
    doc: {type: git, url: 'https://example.org/demo.git', version: main}
    release:
      packages: [demo_a, demo_b]
      tags: {release: 'release/jazzy/{package}/{version}'}
      url: 'https://example.org/demo-release.git'
      version: 1.2.3-1
    source: {type: git, url: 'https://example.org/demo.git', version: main}
    status: maintained
type: distribution
version: 2
Y
for p in demo_a demo_b; do
  d=pkgs/demo_a; [ $p = demo_b ] && d=pkgs/sub/demo_b
  dep=""; [ $p = demo_b ] && dep="<depend>demo_a</depend><exec_depend>rclpy</exec_depend>"
  printf '<?xml version="1.0"?>\n<package format="3"><name>%s</name><version>1.2.3</version><description>d</description><maintainer email="m@example.invalid">m</maintainer><license>Apache-2.0</license>%s</package>\n' "$p" "$dep" > $d/package.xml
done
cat > t.py <<'P'
import os, sys
fails = 0
def check(ok, what):
    global fails
    print(("  ok   " if ok else "  FAIL ") + what); fails += (not ok)
T = os.getcwd()
import rosdistro
idx = rosdistro.get_index("file://%s/rd/index.yaml" % T)
check(list(idx.distributions) == ["jazzy"] and idx.distributions["jazzy"]["distribution_type"] == "ros2", "rosdistro: index with one ros2 distribution")
df = rosdistro.get_distribution_file(idx, "jazzy")
r = df.repositories["demo"]
check(r.release_repository.version == "1.2.3-1" and sorted(r.release_repository.package_names) == ["demo_a", "demo_b"], "rosdistro: release version and packages")
check(r.source_repository.url == "https://example.org/demo.git" and r.status == "maintained", "rosdistro: source repository and status")
check(sorted(df.release_packages) == ["demo_a", "demo_b"] and df.release_platforms == {"ubuntu": ["noble"]}, "rosdistro: release packages and platforms")
import rospkg
rp = rospkg.RosPack(ros_paths=[os.path.join(T, "pkgs")])
check(sorted(rp.list()) == ["demo_a", "demo_b"], "rospkg: RosPack finds both packages")
check(rp.get_path("demo_b") == os.path.join(T, "pkgs", "sub", "demo_b"), "rospkg: get_path")
m = rp.get_manifest("demo_b")
check(m.version == "1.2.3" and m.is_catkin and m.licenses == ["Apache-2.0"], "rospkg: package.xml read as a catkin manifest (version, license)")
sys.exit(1 if fails else 0)
P
"$PY" t.py || fails=$((fails+1))
[ $fails -eq 0 ] && { echo "PASS python3-ros-tools"; exit 0; }
echo "FAIL python3-ros-tools ($fails)"; exit 1
