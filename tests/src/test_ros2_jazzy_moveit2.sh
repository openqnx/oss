#!/bin/sh
# ros2-jazzy-moveit2: move_group plans for a two-joint planar arm.
# The arm (two 1 m links turning about z) is described by an inline URDF and
# SRDF; move_group runs with the OMPL and Pilz pipelines and no controllers.
# Known answers:
#  - /check_state_validity: with a 0.4 m box at (1.5, 0, 0), the stretched
#    arm (0, 0) collides with it (FCL) and the elbow-up arm (0, pi/2) does not;
#  - /plan_kinematic_path with Pilz PTP from (0, 0) to (pi/2, 0) with joint
#    limits v = 1 rad/s, a = 1 rad/s^2: a trapezoid of pi/2 + 1 = 2.5708 s;
#  - /plan_kinematic_path with OMPL (RRTConnect) from (-1.2, 0) to (1.2, 0)
#    around the box: it ends at the goal, every point of the (time-
#    parameterised, path tolerance 0.001 rad) trajectory is collision-free,
#    and it takes at least the straight-line minimum of 2.4 + 1 = 3.4 s.
# (Pilz needs an IK solver for the group: the KDL plugin is configured.)
echo "== ros2-jazzy-moveit2 =="
fail=0
R=/usr/local/opt/ros/jazzy
[ -f "$R/setup.sh" ] || { echo "  FAIL no $R/setup.sh"; echo "FAIL ros2-jazzy-moveit2 (1 check(s) failed)"; exit 1; }
. "$R/setup.sh"
ok()  { echo "  ok   $1"; }
bad() { echo "  FAIL $1"; fail=$((fail + 1)); }
for p in moveit_core moveit_ros_move_group moveit_planners_ompl pilz_industrial_motion_planner moveit_kinematics moveit_ros_planning_interface moveit_servo moveit_planners_chomp; do
  [ -f "$R/share/ament_index/resource_index/packages/$p" ] && ok "ament index has $p" || bad "ament index lacks $p"
done
T=$(mktemp -d /tmp/ros2-moveit-test.XXXXXX)
ROS_LOG_DIR="$T/log"; export ROS_LOG_DIR
ROS_DOMAIN_ID=$(( ($$ % 50) + 50 )); export ROS_DOMAIN_ID
ROS_AUTOMATIC_DISCOVERY_RANGE=LOCALHOST; export ROS_AUTOMATIC_DISCOVERY_RANGE
python3 - "$T" <<'PYEOF'
import sys
t = sys.argv[1]
def link(n):
    return """  <link name="%s"><collision><origin xyz="0.5 0 0"/><geometry><box size="1.0 0.1 0.1"/></geometry></collision>
    <inertial><mass value="1"/><inertia ixx="0.01" iyy="0.01" izz="0.01" ixy="0" ixz="0" iyz="0"/></inertial></link>
""" % n
urdf = """<?xml version="1.0"?>
<robot name="planar2">
  <link name="base_link"/>
%s%s  <link name="tip"/>
  <joint name="joint1" type="revolute"><parent link="base_link"/><child link="link1"/><axis xyz="0 0 1"/>
    <limit lower="-3.1" upper="3.1" effort="10" velocity="1.0"/></joint>
  <joint name="joint2" type="revolute"><parent link="link1"/><child link="link2"/><origin xyz="1 0 0"/><axis xyz="0 0 1"/>
    <limit lower="-3.1" upper="3.1" effort="10" velocity="1.0"/></joint>
  <joint name="tip_joint" type="fixed"><parent link="link2"/><child link="tip"/><origin xyz="1 0 0"/></joint>
</robot>""" % (link("link1"), link("link2"))
srdf = """<?xml version="1.0"?>
<robot name="planar2">
  <group name="arm"><chain base_link="base_link" tip_link="tip"/></group>
  <disable_collisions link1="base_link" link2="link1" reason="Adjacent"/>
  <disable_collisions link1="link1" link2="link2" reason="Adjacent"/>
</robot>"""
ind = lambda s, n: "\n".join(" " * n + l for l in s.splitlines())
lim = "{has_velocity_limits: true, max_velocity: 1.0, has_acceleration_limits: true, max_acceleration: 1.0, has_deceleration_limits: true, max_deceleration: -1.0}"
open(t + "/move_group.yaml", "w").write("""move_group:
  ros__parameters:
    robot_description: |
%s
    robot_description_semantic: |
%s
    planning_pipelines: ["ompl", "pilz_industrial_motion_planner"]
    default_planning_pipeline: ompl
    ompl:
      planning_plugins: ["ompl_interface/OMPLPlanner"]
      request_adapters: ["default_planning_request_adapters/ResolveConstraintFrames", "default_planning_request_adapters/ValidateWorkspaceBounds", "default_planning_request_adapters/CheckStartStateBounds", "default_planning_request_adapters/CheckStartStateCollision"]
      response_adapters: ["default_planning_response_adapters/AddTimeOptimalParameterization", "default_planning_response_adapters/ValidateSolution"]
      totg:
        path_tolerance: 0.001
      planner_configs:
        RRTConnectkConfigDefault:
          type: geometric::RRTConnect
          range: 0.0
      arm:
        planner_configs: ["RRTConnectkConfigDefault"]
        # Check motions finely enough (a thousandth of the joint-space extent) that
        # the box cannot be clipped between checked states.
        longest_valid_segment_fraction: 0.001
    pilz_industrial_motion_planner:
      planning_plugins: ["pilz_industrial_motion_planner/CommandPlanner"]
    robot_description_kinematics:
      arm:
        kinematics_solver: kdl_kinematics_plugin/KDLKinematicsPlugin
        kinematics_solver_search_resolution: 0.005
        kinematics_solver_timeout: 0.05
    robot_description_planning:
      joint_limits:
        joint1: %s
        joint2: %s
      cartesian_limits:
        max_trans_vel: 1.0
        max_trans_acc: 2.25
        max_trans_dec: -5.0
        max_rot_vel: 1.57
    capabilities: ""
    allow_trajectory_execution: false
    moveit_manage_controllers: false
    publish_robot_description_semantic: true
""" % (ind(urdf, 6), ind(srdf, 6), lim, lim))
PYEOF
"$R/lib/moveit_ros_move_group/move_group" --ros-args --params-file "$T/move_group.yaml" > "$T/mg.out" 2>&1 &
mg=$!
cat > "$T/plan.py" <<'PYEOF'
import math, sys, time
import rclpy
from rclpy.node import Node
from moveit_msgs.srv import GetStateValidity, GetMotionPlan, ApplyPlanningScene
from moveit_msgs.msg import RobotState, Constraints, JointConstraint, CollisionObject, PlanningScene
from shape_msgs.msg import SolidPrimitive
from geometry_msgs.msg import Pose
fail = 0
def check(c, what):
    global fail
    print(("  ok   " if c else "  FAIL ") + what, flush=True)
    fail += 0 if c else 1
rclpy.init()
n = Node("qnx_moveit_test")
def call(name, typ, req, t=60.0):
    c = n.create_client(typ, name)
    if not c.wait_for_service(timeout_sec=t): return None
    f = c.call_async(req); rclpy.spin_until_future_complete(n, f, timeout_sec=t); return f.result()
def state(q):
    s = RobotState(); s.joint_state.name = ["joint1", "joint2"]; s.joint_state.position = list(q); return s
box = CollisionObject(); box.id = "box"; box.header.frame_id = "base_link"; box.operation = CollisionObject.ADD
box.primitives = [SolidPrimitive(type=SolidPrimitive.BOX, dimensions=[0.4, 0.4, 0.4])]
p = Pose(); p.position.x = 1.5; p.orientation.w = 1.0; box.primitive_poses = [p]
ps = PlanningScene(is_diff=True); ps.world.collision_objects = [box]
r = call("/apply_planning_scene", ApplyPlanningScene, ApplyPlanningScene.Request(scene=ps), t=90)
check(r is not None and r.success, "move_group is up; a 0.4 m box added at (1.5, 0, 0)")
def valid(q):
    r = call("/check_state_validity", GetStateValidity, GetStateValidity.Request(robot_state=state(q), group_name="arm"))
    return None if r is None else r.valid
v1, v2 = valid((0.0, 0.0)), valid((0.0, math.pi / 2))
check(v1 is False, "check_state_validity: (0, 0) collides with the box")
check(v2 is True, "check_state_validity: (0, pi/2) is collision-free")
def plan(pipeline, planner, q0, q1):
    req = GetMotionPlan.Request()
    m = req.motion_plan_request
    m.group_name = "arm"; m.pipeline_id = pipeline; m.planner_id = planner
    m.start_state = state(q0); m.num_planning_attempts = 1; m.allowed_planning_time = 10.0
    m.max_velocity_scaling_factor = 1.0; m.max_acceleration_scaling_factor = 1.0
    c = Constraints()
    for j, v in zip(("joint1", "joint2"), q1):
        c.joint_constraints.append(JointConstraint(joint_name=j, position=v, tolerance_above=1e-4, tolerance_below=1e-4, weight=1.0))
    m.goal_constraints = [c]
    r = call("/plan_kinematic_path", GetMotionPlan, req)
    return r.motion_plan_response if r else None
t_ = lambda pt: pt.time_from_start.sec + pt.time_from_start.nanosec * 1e-9
r = plan("pilz_industrial_motion_planner", "PTP", (0.0, 0.0), (math.pi / 2, 0.0))
pts = r.trajectory.joint_trajectory.points if r else []
ok = r is not None and r.error_code.val == 1 and pts
d = t_(pts[-1]) if ok else -1
check(ok and abs(d - (math.pi / 2 + 1.0)) < 0.02 and abs(pts[-1].positions[0] - math.pi / 2) < 1e-4,
      "Pilz PTP (0,0) -> (pi/2,0): %.4f s (trapezoid pi/2 + 1 = %.4f s), error code %s" % (d, math.pi / 2 + 1, r.error_code.val if r else None))
# RRTConnect is randomised: now and then the time parameterisation's corner
# blending pushes a path that hugs the box a hair into it, and ValidateSolution
# rejects it (error code 99999). A user would plan again; so does the test, up
# to three times, and says which attempt succeeded.
for attempt in (1, 2, 3):
    r = plan("ompl", "RRTConnectkConfigDefault", (-1.2, 0.0), (1.2, 0.0))
    pts = r.trajectory.joint_trajectory.points if r else []
    if r is not None and r.error_code.val == 1 and len(pts) > 2:
        break
check(r is not None and r.error_code.val == 1 and len(pts) > 2, "OMPL RRTConnect found a plan around the box (%d points, error code %s, attempt %d of 3)" % (len(pts), r.error_code.val if r else None, attempt))
if pts:
    e = pts[-1].positions
    check(abs(e[0] - 1.2) < 1e-3 and abs(e[1]) < 1e-3, "it ends at the goal (1.2, 0) (got (%.4f, %.4f))" % (e[0], e[1]))
    bad_pts = [i for i, pt in enumerate(pts) if valid(tuple(pt.positions)) is not True]
    check(not bad_pts, "every trajectory point is collision-free (%d checked)" % len(pts))
    check(t_(pts[-1]) >= 3.4 - 1e-3, "it is time-parameterised: %.3f s >= the 3.4 s straight-line minimum" % t_(pts[-1]))
rclpy.shutdown()
sys.exit(1 if fail else 0)
PYEOF
python3 "$T/plan.py" > "$T/plan.out" 2>&1
rc=$?
grep -E '^  (ok|FAIL) ' "$T/plan.out"
if [ $rc -ne 0 ]; then
  n=$(grep -c '^  FAIL ' "$T/plan.out"); [ "$n" -gt 0 ] || n=1; fail=$((fail + n))
  grep -vE '^  (ok|FAIL) ' "$T/plan.out" | tail -5 | sed 's/^/       python: /'
  grep -iE "error|fatal|fail|terminate|what\(\)" "$T/mg.out" | tail -8 | sed 's/^/       move_group: /'
fi
kill $mg 2>/dev/null; sleep 2
# A node that does not exit on SIGTERM must not hold the test up.
kill -9 $mg 2>/dev/null; wait $mg 2>/dev/null
rm -rf "$T"
if [ $fail -eq 0 ]; then echo "PASS ros2-jazzy-moveit2"; exit 0; fi
echo "FAIL ros2-jazzy-moveit2 ($fail check(s) failed)"; exit 1
