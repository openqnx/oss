#!/bin/sh
# ROS 2: demo_nodes_cpp's talker and listener in two processes, once over each
# DDS middleware built (Fast DDS, Cyclone DDS). The listener must report
# messages from the talker: node creation, type support lookup through the
# ament index, discovery and transport all have to work for that.
echo "== ros2 =="
fail=0
P="$AMENT_PREFIX_PATH"
if [ -z "$P" ] || [ ! -x "$P/lib/demo_nodes_cpp/talker" ]; then
  echo "  FAIL ROS prefix with demo_nodes_cpp (AMENT_PREFIX_PATH=$P)"
  echo "FAIL ros2 (1 check(s) failed)"; exit 1
fi
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
rm -rf "$T"
if [ $fail -eq 0 ]; then echo "PASS ros2"; exit 0; fi
echo "FAIL ros2 ($fail check(s) failed)"; exit 1
