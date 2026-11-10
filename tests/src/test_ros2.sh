#!/bin/sh
# ROS 2: demo_nodes_cpp's talker and listener in two processes, once over each
# DDS middleware built (Fast DDS, Cyclone DDS). The listener must report
# messages from the talker: node creation, type support lookup through the
# ament index, discovery and transport all have to work for that.
# Then the Python side (rclpy): it imports with its run-time dependencies
# (PyYAML, NumPy); a String published by one node arrives at another; a
# message with a fixed-size array field holds it as a NumPy array; the tf2_py
# C module computes a known transform chain; and a tf2_ros
# StaticTransformBroadcaster and TransformBroadcaster are looked up through a
# TransformListener with the known composed result.
echo "== ros2 =="
fail=0
P="$AMENT_PREFIX_PATH"
if [ -z "$P" ] || [ ! -x "$P/lib/demo_nodes_cpp/talker" ]; then
  echo "  FAIL ROS prefix with demo_nodes_cpp (AMENT_PREFIX_PATH=$P)"
  echo "FAIL ros2 (1 check(s) failed)"; exit 1
fi
# ROS's own libraries first, as its setup.sh puts them: the ROS build carries
# its own Fast DDS, Fast CDR, Cyclone DDS, zstd, yaml-cpp ... under the same
# sonames as this tree's standalone components in /usr/local/lib, and QNX's
# loader takes LD_LIBRARY_PATH before any RPATH -- with the standalone
# libfastcdr loaded, every Fast DDS node threw BadParamException ("This member
# is not been selected") while registering its first type.
LD_LIBRARY_PATH="$P/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"; export LD_LIBRARY_PATH
T=$(mktemp -d /tmp/ros2-test.XXXXXX)
ROS_LOG_DIR="$T/log"; export ROS_LOG_DIR
ROS_DOMAIN_ID=$(( $$ % 200 )); export ROS_DOMAIN_ID
# Discovery on this host only: a test VM's other interfaces are of no use here.
ROS_AUTOMATIC_DISCOVERY_RANGE=LOCALHOST; export ROS_AUTOMATIC_DISCOVERY_RANGE

for rmw in rmw_fastrtps_cpp rmw_cyclonedds_cpp; do
  RMW_IMPLEMENTATION=$rmw; export RMW_IMPLEMENTATION
  "$P/lib/demo_nodes_cpp/listener" > "$T/listener.$rmw" 2>&1 &
  lp=$!
  "$P/lib/demo_nodes_cpp/talker" > "$T/talker.$rmw" 2>&1 &
  tp=$!
  i=0
  while [ $i -lt 20 ] && ! grep -q 'I heard' "$T/listener.$rmw"; do sleep 1; i=$((i + 1)); done
  kill $tp $lp 2>/dev/null; wait $tp $lp 2>/dev/null
  if grep -q 'I heard' "$T/listener.$rmw"; then
    echo "  ok   talker -> listener over $rmw ($(grep -c 'I heard' "$T/listener.$rmw") messages)"
  else
    echo "  FAIL talker -> listener over $rmw"
    sed 's/^/       talker: /' "$T/talker.$rmw" | tail -5
    sed 's/^/       listener: /' "$T/listener.$rmw" | tail -5
    fail=$((fail + 1))
  fi
done
# ---- Python: rclpy, generated messages, tf2_py and tf2_ros ----
PY=$(command -v python3 || echo /usr/local/bin/python3)
SP=$(ls -d "$P"/lib/python3*/site-packages 2>/dev/null | head -1)
PYTHONPATH="$SP${PYTHONPATH:+:$PYTHONPATH}"; export PYTHONPATH
unset RMW_IMPLEMENTATION
cat > "$T/rclpy_test.py" <<'PYEOF'
import math, sys, time
fail = 0
def check(cond, what):
    global fail
    print(("  ok   " if cond else "  FAIL ") + what, flush=True)
    if not cond:
        fail += 1
try:
    import yaml, numpy
    import rclpy
    from rclpy.node import Node
    from rclpy.executors import SingleThreadedExecutor
    from std_msgs.msg import String
    from geometry_msgs.msg import PoseWithCovariance, TransformStamped
    import tf2_py
    import tf2_ros
    from tf2_ros import Buffer, TransformListener, TransformBroadcaster, StaticTransformBroadcaster
    check(True, "import rclpy, std_msgs/geometry_msgs messages, tf2_py, tf2_ros (with yaml, numpy)")
except Exception as e:
    check(False, "imports: %r" % (e,))
    sys.exit(1)

# A fixed-size array field is a NumPy array.
m = PoseWithCovariance()
m.covariance[7] = 2.5
check(isinstance(m.covariance, numpy.ndarray) and m.covariance.shape == (36,) and m.covariance.sum() == 2.5,
      "PoseWithCovariance.covariance is a 36-element NumPy array")

# tf2_py alone: world -> a (translate x 1), a -> b (rotate 90 deg about z, translate y 2).
def tfm(parent, child, t, q, stamp_sec=0):
    x = TransformStamped()
    x.header.frame_id = parent; x.child_frame_id = child
    x.header.stamp.sec = stamp_sec
    x.transform.translation.x, x.transform.translation.y, x.transform.translation.z = t
    x.transform.rotation.x, x.transform.rotation.y, x.transform.rotation.z, x.transform.rotation.w = q
    return x
s2 = math.sqrt(0.5)
core = tf2_py.BufferCore()
core.set_transform_static(tfm("world", "a", (1.0, 0.0, 0.0), (0.0, 0.0, 0.0, 1.0)), "test")
core.set_transform_static(tfm("a", "b", (0.0, 2.0, 0.0), (0.0, 0.0, s2, s2)), "test")
from builtin_interfaces.msg import Time
r = core.lookup_transform_core("world", "b", Time())
t = r.transform.translation; q = r.transform.rotation
check(abs(t.x - 1) < 1e-9 and abs(t.y - 2) < 1e-9 and abs(t.z) < 1e-9 and abs(q.z - s2) < 1e-9 and abs(q.w - s2) < 1e-9,
      "tf2_py BufferCore: world->b = (1, 2, 0), 90 deg about z (got (%.3f, %.3f, %.3f))" % (t.x, t.y, t.z))
r = core.lookup_transform_core("b", "world", Time())
t = r.transform.translation
# inverse: R^-1 * -(1,2,0) with R = 90 deg about z -> (-2, 1, 0)
check(abs(t.x + 2) < 1e-9 and abs(t.y - 1) < 1e-9, "tf2_py BufferCore: b->world = (-2, 1, 0) (got (%.3f, %.3f, %.3f))" % (t.x, t.y, t.z))

rclpy.init()
pub_node = Node("qnx_test_talker_py"); sub_node = Node("qnx_test_listener_py")
got = []
sub_node.create_subscription(String, "qnx_test_chatter_py", lambda msg: got.append(msg.data), 10)
pub = pub_node.create_publisher(String, "qnx_test_chatter_py", 10)
ex = SingleThreadedExecutor(); ex.add_node(pub_node); ex.add_node(sub_node)
end = time.time() + 20; n = 0
while time.time() < end and not got:
    msg = String(); msg.data = "hello from QNX %d" % n; n += 1
    pub.publish(msg)
    ex.spin_once(timeout_sec=0.2)
for _ in range(5): ex.spin_once(timeout_sec=0.05)
check(bool(got) and got[0].startswith("hello from QNX "), "rclpy: String published by one node received by another (%r)" % (got[:1],))

# tf2_ros: a static world->base (1, 2, 0) and a dynamic base->tool (0, 0, 0.5, 90 deg about z),
# looked up as world->tool through a TransformListener.
bnode = Node("qnx_test_tf_broadcaster")
sb = StaticTransformBroadcaster(bnode)
sb.sendTransform(tfm("world", "base", (1.0, 2.0, 0.0), (0.0, 0.0, 0.0, 1.0)))
db = TransformBroadcaster(bnode)
lnode = Node("qnx_test_tf_listener")
buf = Buffer(); lst = TransformListener(buf, lnode, spin_thread=False)
ex.add_node(bnode); ex.add_node(lnode)
res = None; end = time.time() + 20
while time.time() < end and res is None:
    x = tfm("base", "tool", (0.0, 0.0, 0.5), (0.0, 0.0, s2, s2))
    x.header.stamp = bnode.get_clock().now().to_msg()
    db.sendTransform(x)
    ex.spin_once(timeout_sec=0.1)
    if buf.can_transform("world", "tool", rclpy.time.Time()):
        res = buf.lookup_transform("world", "tool", rclpy.time.Time())
if res is not None:
    t = res.transform.translation; q = res.transform.rotation
    check(abs(t.x - 1) < 1e-9 and abs(t.y - 2) < 1e-9 and abs(t.z - 0.5) < 1e-9 and abs(q.z - s2) < 1e-6 and abs(q.w - s2) < 1e-6,
          "tf2_ros: world->tool = (1, 2, 0.5), 90 deg about z (got (%.3f, %.3f, %.3f) q.z %.4f)" % (t.x, t.y, t.z, q.z))
else:
    check(False, "tf2_ros: no world->tool transform within 20 s (frames: %s)" % buf.all_frames_as_string().strip())
rclpy.shutdown()
sys.exit(1 if fail else 0)
PYEOF
"$PY" "$T/rclpy_test.py" > "$T/py.out" 2>&1
rc=$?
cat "$T/py.out" | grep -E '^  (ok|FAIL) '
if [ $rc -ne 0 ]; then
  n=$(grep -c '^  FAIL ' "$T/py.out"); [ "$n" -gt 0 ] || n=1
  fail=$((fail + n))
  grep -vE '^  (ok|FAIL) ' "$T/py.out" | tail -8 | sed 's/^/       python: /'
fi
rm -rf "$T"
if [ $fail -eq 0 ]; then echo "PASS ros2"; exit 0; fi
echo "FAIL ros2 ($fail check(s) failed)"; exit 1
