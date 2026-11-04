#!/bin/sh
# ros2-rviz (headless part; drawing is checked on a Screen target, see
# ports/ros2-rviz/README): the RViz packages are in the ROS 2 install and its
# ament index; rviz2 starts -- its libraries (rviz_common, rviz_rendering,
# OGRE, Qt 6) load and resolve -- and prints its command-line help with the
# options RViz 14 has; the default plugins' descriptions list the displays a
# Grid/TF/Marker view needs, and their library and OGRE's GLES2 render
# system and RTShader library are installed.
echo "== ros2-rviz =="
fail=0
R=/usr/local/opt/ros/jazzy
ok()  { echo "  ok   $1"; }
bad() { echo "  FAIL $1"; fail=$((fail + 1)); }
[ -f "$R/setup.sh" ] || { echo "  FAIL no $R/setup.sh"; echo "FAIL ros2-rviz (1 check(s) failed)"; exit 1; }
# ROS's own libraries first, as its setup.sh puts them (see test_ros2.sh).
. "$R/setup.sh"
case ":$LD_LIBRARY_PATH:" in *":$R/opt/rviz_ogre_vendor/lib:"*) ok "setup.sh puts OGRE's libraries on LD_LIBRARY_PATH" ;;
  *) bad "OGRE's lib directory not on LD_LIBRARY_PATH ($LD_LIBRARY_PATH)" ;; esac

for p in rviz2 rviz_common rviz_rendering rviz_default_plugins rviz_ogre_vendor rviz_assimp_vendor interactive_markers; do
  if [ -f "$R/share/ament_index/resource_index/packages/$p" ]; then ok "ament index has $p"; else bad "ament index lacks $p"; fi
done

T=$(mktemp -d /tmp/rviz-test.XXXXXX)
ROS_LOG_DIR="$T/log"; export ROS_LOG_DIR
# No display on this target: Qt's offscreen platform is enough for --help.
QT_QPA_PLATFORM=offscreen "$R/lib/rviz2/rviz2" --help > "$T/help" 2>&1
rc=$?
if [ $rc -eq 0 ] && grep -q -- '--display-config' "$T/help" && grep -q -- '--fixed-frame' "$T/help"; then
  ok "rviz2 --help lists --display-config and --fixed-frame"
else
  bad "rviz2 --help (rc=$rc)"; sed 's/^/       /' "$T/help" | tail -8
fi

X="$R/share/rviz_default_plugins/plugins_description.xml"
for c in rviz_default_plugins/Grid rviz_default_plugins/TF rviz_default_plugins/Marker rviz_default_plugins/PointCloud2; do
  if grep -q "name=\"$c\"" "$X" 2>/dev/null; then ok "plugin $c described"; else bad "plugin $c not in $X"; fi
done
[ -f "$R/lib/librviz_default_plugins.so" ] && ok "librviz_default_plugins.so installed" || bad "no librviz_default_plugins.so"
[ -f "$R/opt/rviz_ogre_vendor/lib/OGRE/RenderSystem_GLES2.so" ] && ok "OGRE RenderSystem_GLES2 installed" || bad "no RenderSystem_GLES2.so"
[ -f "$R/opt/rviz_ogre_vendor/share/OGRE/Media/RTShaderLib/GLSL/FFPLib_Transform.glsl" ] && ok "RTShader library installed" || bad "no RTShader library"
# One copy of OGRE (in opt/rviz_ogre_vendor), not a second in the prefix's lib.
[ ! -e "$R/lib/libOgreMain.so.1.12.10" ] && ok "no second libOgreMain in $R/lib" || bad "a second libOgreMain in $R/lib"

rm -rf "$T"
if [ $fail -eq 0 ]; then echo "PASS ros2-rviz"; exit 0; fi
echo "FAIL ros2-rviz ($fail check(s) failed)"; exit 1
