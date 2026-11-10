#!/bin/sh
# ros2-jazzy-nav2: plan a path with Nav2's servers on a known map.
# A 5 m x 5 m occupancy grid (0.05 m cells) has a wall at x = 2.4..2.6 m
# from y = 0 up to 4 m; the only way from (1, 1) to (4, 1) is over its top.
# nav2_map_server serves it, nav2_planner_server (NavFn plugin, global
# costmap with static and inflation layers) plans, nav2_lifecycle_manager
# brings both up; a static map -> base_link transform stands in for
# localisation. The ComputePathToPose action must return a path that starts
# and ends at the requested poses, passes above y = 4 m, never enters the
# wall, and is longer than the 3 m straight line (at least 2 * |(1.4, 3)| + 0.2).
# The navigator's default behaviour tree (BT.CPP 4 XML) must be installed.
echo "== ros2-jazzy-nav2 =="
fail=0
R=/usr/local/opt/ros/jazzy
[ -f "$R/setup.sh" ] || { echo "  FAIL no $R/setup.sh"; echo "FAIL ros2-jazzy-nav2 (1 check(s) failed)"; exit 1; }
. "$R/setup.sh"
ok()  { echo "  ok   $1"; }
bad() { echo "  FAIL $1"; fail=$((fail + 1)); }
for p in nav2_map_server nav2_planner nav2_navfn_planner nav2_controller nav2_bt_navigator nav2_costmap_2d nav2_amcl nav2_mppi_controller nav2_smac_planner robot_localization behaviortree_cpp; do
  [ -f "$R/share/ament_index/resource_index/packages/$p" ] && ok "ament index has $p" || bad "ament index lacks $p"
done
T=$(mktemp -d /tmp/ros2-nav2-test.XXXXXX)
ROS_LOG_DIR="$T/log"; export ROS_LOG_DIR
ROS_DOMAIN_ID=$(( ($$ % 50) + 150 )); export ROS_DOMAIN_ID
ROS_AUTOMATIC_DISCOVERY_RANGE=LOCALHOST; export ROS_AUTOMATIC_DISCOVERY_RANGE
python3 - "$T" <<'PYEOF'
import sys
t = sys.argv[1]
W = H = 100
rows = []
for r in range(H):            # PGM row 0 is the top (y = 5 m)
    y = (H - 1 - r) * 0.05 + 0.025
    row = bytearray()
    for c in range(W):
        x = c * 0.05 + 0.025
        row.append(0 if (2.4 <= x <= 2.6 and y <= 4.0) else 254)
    rows.append(bytes(row))
open(t + "/map.pgm", "wb").write(b"P5\n%d %d\n255\n" % (W, H) + b"".join(rows))
open(t + "/map.yaml", "w").write("image: map.pgm\nresolution: 0.05\norigin: [0.0, 0.0, 0.0]\nnegate: 0\noccupied_thresh: 0.65\nfree_thresh: 0.196\nmode: trinary\n")
open(t + "/nav2.yaml", "w").write("""map_server:
  ros__parameters:
    yaml_filename: %s/map.yaml
planner_server:
  ros__parameters:
    expected_planner_frequency: 1.0
    planner_plugins: ["GridBased"]
    GridBased:
      plugin: "nav2_navfn_planner::NavfnPlanner"
      tolerance: 0.1
      use_astar: true
      allow_unknown: false
global_costmap:
  global_costmap:
    ros__parameters:
      global_frame: map
      robot_base_frame: base_link
      update_frequency: 1.0
      publish_frequency: 1.0
      resolution: 0.05
      robot_radius: 0.1
      track_unknown_space: false
      plugins: ["static_layer", "inflation_layer"]
      static_layer:
        plugin: "nav2_costmap_2d::StaticLayer"
        map_subscribe_transient_local: True
      inflation_layer:
        plugin: "nav2_costmap_2d::InflationLayer"
        cost_scaling_factor: 3.0
        inflation_radius: 0.3
lifecycle_manager:
  ros__parameters:
    autostart: true
    bond_timeout: 0.0
    node_names: ["map_server", "planner_server"]
""" % t)
PYEOF
"$R/lib/tf2_ros/static_transform_publisher" --frame-id map --child-frame-id base_link --x 1 --y 1 > "$T/tf.out" 2>&1 &
p0=$!
"$R/lib/nav2_map_server/map_server" --ros-args --params-file "$T/nav2.yaml" > "$T/map.out" 2>&1 &
p1=$!
"$R/lib/nav2_planner/planner_server" --ros-args --params-file "$T/nav2.yaml" > "$T/planner.out" 2>&1 &
p2=$!
"$R/lib/nav2_lifecycle_manager/lifecycle_manager" --ros-args -r __node:=lifecycle_manager --params-file "$T/nav2.yaml" > "$T/lm.out" 2>&1 &
p3=$!
cat > "$T/plan.py" <<'PYEOF'
import math, sys, time
import rclpy
from rclpy.node import Node
from rclpy.action import ActionClient
from nav2_msgs.action import ComputePathToPose
fail = 0
def check(c, what):
    global fail
    print(("  ok   " if c else "  FAIL ") + what, flush=True)
    fail += 0 if c else 1
rclpy.init()
n = Node("qnx_nav2_test")
# Wait for the lifecycle manager to report both servers active.
from std_srvs.srv import Trigger
cli = n.create_client(Trigger, "/lifecycle_manager/is_active")
active = False; end = time.time() + 60
while time.time() < end and not active:
    if cli.wait_for_service(timeout_sec=1.0):
        f = cli.call_async(Trigger.Request()); rclpy.spin_until_future_complete(n, f, timeout_sec=5)
        active = bool(f.result() and f.result().success)
    if not active: time.sleep(0.5)
check(active, "lifecycle_manager brought map_server and planner_server up")
ac = ActionClient(n, ComputePathToPose, "/compute_path_to_pose")
ok = ac.wait_for_server(timeout_sec=20)
check(ok, "compute_path_to_pose action server is there")
res = None
if ok:
    g = ComputePathToPose.Goal()
    g.planner_id = "GridBased"; g.use_start = True
    for p, (x, y) in ((g.start, (1.0, 1.0)), (g.goal, (4.0, 1.0))):
        p.header.frame_id = "map"; p.pose.position.x = x; p.pose.position.y = y; p.pose.orientation.w = 1.0
    end = time.time() + 30
    while time.time() < end and res is None:   # the costmap needs the map first
        f = ac.send_goal_async(g); rclpy.spin_until_future_complete(n, f, timeout_sec=10)
        gh = f.result()
        if gh and gh.accepted:
            rf = gh.get_result_async(); rclpy.spin_until_future_complete(n, rf, timeout_sec=20)
            r = rf.result()
            if r and r.result.path.poses: res = r.result
        if res is None: time.sleep(1)
poses = [(p.pose.position.x, p.pose.position.y) for p in res.path.poses] if res else []
check(len(poses) > 10, "a path was planned (%d poses)" % len(poses))
if poses:
    L = sum(math.hypot(b[0] - a[0], b[1] - a[1]) for a, b in zip(poses, poses[1:]))
    s, e = poses[0], poses[-1]
    check(math.hypot(s[0] - 1, s[1] - 1) < 0.1 and math.hypot(e[0] - 4, e[1] - 1) < 0.11,
          "it starts at (1, 1) and ends at (4, 1) (got (%.2f, %.2f) .. (%.2f, %.2f))" % (s + e))
    top = max(y for _, y in poses)
    check(top > 4.0, "it goes over the wall (highest y %.2f m > 4)" % top)
    inside = [p for p in poses if 2.4 <= p[0] <= 2.6 and p[1] <= 4.0]
    check(not inside, "no pose inside the wall")
    check(L >= 2 * math.hypot(1.4, 3.0) + 0.2 - 0.05, "length %.2f m >= the shortest way round, %.2f m" % (L, 2 * math.hypot(1.4, 3.0) + 0.2))
rclpy.shutdown()
sys.exit(1 if fail else 0)
PYEOF
python3 "$T/plan.py" > "$T/plan.out" 2>&1
rc=$?
grep -E '^  (ok|FAIL) ' "$T/plan.out"
if [ $rc -ne 0 ]; then
  n=$(grep -c '^  FAIL ' "$T/plan.out"); [ "$n" -gt 0 ] || n=1; fail=$((fail + n))
  grep -vE '^  (ok|FAIL) ' "$T/plan.out" | tail -5 | sed 's/^/       python: /'
  for f in planner map lm; do tail -4 "$T/$f.out" | sed "s/^/       $f: /"; done
fi
kill $p3 $p2 $p1 $p0 2>/dev/null; sleep 2
# A node that does not exit on SIGTERM must not hold the test up.
kill -9 $p3 $p2 $p1 $p0 2>/dev/null; wait $p3 $p2 $p1 $p0 2>/dev/null
X="$R/share/nav2_bt_navigator/behavior_trees/navigate_to_pose_w_replanning_and_recovery.xml"
if [ -f "$X" ] && grep -q 'ComputePathToPose' "$X" && grep -q 'BTCPP_format="4"' "$X"; then
  ok "the navigator's default behaviour tree (BT.CPP 4 format) is installed"
else
  bad "no BT.CPP 4 default behaviour tree at $X"
fi
rm -rf "$T"
if [ $fail -eq 0 ]; then echo "PASS ros2-jazzy-nav2"; exit 0; fi
echo "FAIL ros2-jazzy-nav2 ($fail check(s) failed)"; exit 1
