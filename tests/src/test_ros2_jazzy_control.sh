#!/bin/sh
# ros2-jazzy-control: a ros2_control system with mock hardware, end to end.
# robot_state_publisher publishes a one-joint URDF whose <ros2_control> tag
# names mock_components/GenericSystem; ros2_control_node (controller_manager)
# loads it from /robot_description; the Python spawner loads and activates
# joint_state_broadcaster and a forward_command_controller; a position
# command of 0.5 rad then comes back on /joint_states (the mock hardware
# mirrors commands to states), and a joint_trajectory_controller (switched
# in for the forward controller) drives the joint to -0.8 rad. Known answers
# all the way: the controller list and states, and both joint positions.
echo "== ros2-jazzy-control =="
fail=0
R=/usr/local/opt/ros/jazzy
[ -f "$R/setup.sh" ] || { echo "  FAIL no $R/setup.sh"; echo "FAIL ros2-jazzy-control (1 check(s) failed)"; exit 1; }
. "$R/setup.sh"
ok()  { echo "  ok   $1"; }
bad() { echo "  FAIL $1"; fail=$((fail + 1)); }
for p in controller_manager hardware_interface joint_state_broadcaster forward_command_controller joint_trajectory_controller diff_drive_controller; do
  [ -f "$R/share/ament_index/resource_index/packages/$p" ] && ok "ament index has $p" || bad "ament index lacks $p"
done
T=$(mktemp -d /tmp/ros2-control-test.XXXXXX)
ROS_LOG_DIR="$T/log"; export ROS_LOG_DIR
ROS_DOMAIN_ID=$(( ($$ % 100) + 100 )); export ROS_DOMAIN_ID
ROS_AUTOMATIC_DISCOVERY_RANGE=LOCALHOST; export ROS_AUTOMATIC_DISCOVERY_RANGE
cat > "$T/rsp.yaml" <<'YEOF'
robot_state_publisher:
  ros__parameters:
    robot_description: |
      <?xml version="1.0"?>
      <robot name="qnx_test_bot">
        <link name="base"/>
        <link name="arm"/>
        <joint name="joint1" type="revolute">
          <parent link="base"/><child link="arm"/><axis xyz="0 0 1"/>
          <limit lower="-3" upper="3" effort="10" velocity="10"/>
        </joint>
        <ros2_control name="MockSystem" type="system">
          <hardware><plugin>mock_components/GenericSystem</plugin></hardware>
          <joint name="joint1">
            <command_interface name="position"/>
            <state_interface name="position"><param name="initial_value">0.0</param></state_interface>
            <state_interface name="velocity"/>
          </joint>
        </ros2_control>
      </robot>
YEOF
cat > "$T/cm.yaml" <<'YEOF'
controller_manager:
  ros__parameters:
    update_rate: 100
    joint_state_broadcaster:
      type: joint_state_broadcaster/JointStateBroadcaster
    forward_position_controller:
      type: forward_command_controller/ForwardCommandController
    joint_trajectory_controller:
      type: joint_trajectory_controller/JointTrajectoryController
forward_position_controller:
  ros__parameters:
    joints: [joint1]
    interface_name: position
joint_trajectory_controller:
  ros__parameters:
    joints: [joint1]
    command_interfaces: [position]
    state_interfaces: [position, velocity]
YEOF
"$R/lib/robot_state_publisher/robot_state_publisher" --ros-args --params-file "$T/rsp.yaml" > "$T/rsp.out" 2>&1 &
p1=$!
"$R/lib/controller_manager/ros2_control_node" --ros-args --params-file "$T/cm.yaml" > "$T/cm.out" 2>&1 &
p2=$!
SP="$R/lib/controller_manager/spawner"
# Generous service timeouts: on an emulated (QEMU TCG) aarch64 target, loading
# a controller plugin takes longer than the spawner's default 10 s.
if python3 "$SP" joint_state_broadcaster forward_position_controller -c /controller_manager --controller-manager-timeout 120 --service-call-timeout 120 > "$T/spawn.out" 2>&1; then
  ok "spawner loaded and activated joint_state_broadcaster and forward_position_controller"
else
  bad "spawner"; tail -5 "$T/spawn.out" | sed 's/^/       /'; tail -8 "$T/cm.out" | sed 's/^/       cm: /'
fi
cat > "$T/drive.py" <<'PYEOF'
import sys, time
import rclpy
from rclpy.node import Node
from rclpy.action import ActionClient
from std_msgs.msg import Float64MultiArray
from sensor_msgs.msg import JointState
from controller_manager_msgs.srv import ListControllers, SwitchController, LoadController, ConfigureController
from control_msgs.action import FollowJointTrajectory
from trajectory_msgs.msg import JointTrajectoryPoint
from builtin_interfaces.msg import Duration
fail = 0
def check(c, what):
    global fail
    print(("  ok   " if c else "  FAIL ") + what, flush=True)
    fail += 0 if c else 1
rclpy.init()
n = Node("qnx_control_test")
pos = {}
n.create_subscription(JointState, "/joint_states", lambda m: pos.update(dict(zip(m.name, m.position))), 10)
def call(cli, req, t=30.0):
    if not cli.wait_for_service(timeout_sec=t): return None
    f = cli.call_async(req); rclpy.spin_until_future_complete(n, f, timeout_sec=t); return f.result()
def wait_pos(target, t=30.0):
    end = time.time() + t
    while time.time() < end:
        rclpy.spin_once(n, timeout_sec=0.05)
        if "joint1" in pos and abs(pos["joint1"] - target) < 1e-6: return True
    return False
# The trajectory controller is loaded and configured through the controller
# manager's services (one Python process less than a second spawner run:
# each costs much on an emulated target).
r = call(n.create_client(LoadController, "/controller_manager/load_controller"), LoadController.Request(name="joint_trajectory_controller"))
r2 = call(n.create_client(ConfigureController, "/controller_manager/configure_controller"), ConfigureController.Request(name="joint_trajectory_controller")) if r and r.ok else None
check(r2 is not None and r2.ok, "load_controller + configure_controller: joint_trajectory_controller (inactive)")
lc = n.create_client(ListControllers, "/controller_manager/list_controllers")
r = call(lc, ListControllers.Request())
st = {c.name: c.state for c in r.controller} if r else {}
check(st.get("joint_state_broadcaster") == "active" and st.get("forward_position_controller") == "active"
      and st.get("joint_trajectory_controller") == "inactive", "list_controllers: %s" % sorted(st.items()))
check(wait_pos(0.0), "/joint_states reports joint1 at its initial 0.0")
pub = n.create_publisher(Float64MultiArray, "/forward_position_controller/commands", 10)
end = time.time() + 60   # discovery of the new publisher is slow on an emulated target
while time.time() < end and not (abs(pos.get("joint1", 9) - 0.5) < 1e-6):
    pub.publish(Float64MultiArray(data=[0.5])); rclpy.spin_once(n, timeout_sec=0.1)
check(abs(pos.get("joint1", 9) - 0.5) < 1e-6, "forward command 0.5 -> joint1 at %.6f" % pos.get("joint1", float("nan")))
sw = n.create_client(SwitchController, "/controller_manager/switch_controller")
q = SwitchController.Request(activate_controllers=["joint_trajectory_controller"],
                             deactivate_controllers=["forward_position_controller"], strictness=2)
r = call(sw, q)
check(r is not None and r.ok, "switch_controller: forward_position_controller -> joint_trajectory_controller")
ac = ActionClient(n, FollowJointTrajectory, "/joint_trajectory_controller/follow_joint_trajectory")
ok = ac.wait_for_server(timeout_sec=10)
g = FollowJointTrajectory.Goal()
g.trajectory.joint_names = ["joint1"]
g.trajectory.points = [JointTrajectoryPoint(positions=[-0.8], time_from_start=Duration(sec=1))]
res = None
if ok:
    f = ac.send_goal_async(g); rclpy.spin_until_future_complete(n, f, timeout_sec=10)
    gh = f.result()
    if gh and gh.accepted:
        rf = gh.get_result_async(); rclpy.spin_until_future_complete(n, rf, timeout_sec=15); res = rf.result()
check(res is not None and res.result.error_code == 0, "FollowJointTrajectory goal to -0.8 rad in 1 s succeeded")
check(wait_pos(-0.8, 5), "joint_trajectory_controller -> joint1 at %.6f (-0.8)" % pos.get("joint1", float("nan")))
rclpy.shutdown()
sys.exit(1 if fail else 0)
PYEOF
python3 "$T/drive.py" > "$T/drive.out" 2>&1
rc=$?
grep -E '^  (ok|FAIL) ' "$T/drive.out"
if [ $rc -ne 0 ]; then
  n=$(grep -c '^  FAIL ' "$T/drive.out"); [ "$n" -gt 0 ] || n=1; fail=$((fail + n))
  grep -vE '^  (ok|FAIL) ' "$T/drive.out" | tail -6 | sed 's/^/       python: /'
  tail -6 "$T/cm.out" | sed 's/^/       cm: /'
fi
kill $p2 $p1 2>/dev/null; sleep 2
# A node that does not exit on SIGTERM must not hold the test up.
kill -9 $p2 $p1 2>/dev/null; wait $p2 $p1 2>/dev/null
rm -rf "$T"
if [ $fail -eq 0 ]; then echo "PASS ros2-jazzy-control"; exit 0; fi
echo "FAIL ros2-jazzy-control ($fail check(s) failed)"; exit 1
