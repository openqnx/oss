#!/bin/sh
# run-rviz-demo.sh: the publisher in the background, then rviz2 with demo.rviz
# on Screen (QT_QPA_PLATFORM=qnx); both stop with this script.
D=$(cd "$(dirname "$0")" && pwd)
. /usr/local/opt/ros/jazzy/setup.sh
export ROS_LOG_DIR=/tmp/rviz-demo-log ROS_AUTOMATIC_DISCOVERY_RANGE=LOCALHOST ROS_DOMAIN_ID=${ROS_DOMAIN_ID:-42}
export QT_QPA_PLATFORM=qnx
python3 "$D/rviz_demo_pub.py" > /tmp/rviz-demo-pub.log 2>&1 &
P=$!
rviz2 -d "$D/demo.rviz" &
V=$!
trap 'kill $V $P 2>/dev/null; exit 0' TERM INT
wait $V
kill $P 2>/dev/null
