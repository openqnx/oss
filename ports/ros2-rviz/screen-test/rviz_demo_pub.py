#!/usr/bin/env python3
# Publishes what the screen test shows in RViz: TF map -> base_link (turning
# slowly, 1 m out on x) and base_link -> sensor, a red cube Marker on base_link
# and a green sphere Marker at the map origin, 10 times a second.
import math
import rclpy
from rclpy.node import Node
from geometry_msgs.msg import TransformStamped
from visualization_msgs.msg import Marker
from tf2_msgs.msg import TFMessage


class DemoPub(Node):
    def __init__(self):
        super().__init__('rviz_demo_pub')
        # /tf directly (tf2_ros' Python broadcaster needs tf2_py's C module)
        self.tf = self.create_publisher(TFMessage, '/tf', 10)
        self.markers = self.create_publisher(Marker, 'visualization_marker', 10)
        self.t = 0.0
        self.create_timer(0.1, self.tick)

    def transform(self, parent, child, x, y, z, yaw):
        t = TransformStamped()
        t.header.stamp = self.get_clock().now().to_msg()
        t.header.frame_id = parent
        t.child_frame_id = child
        t.transform.translation.x, t.transform.translation.y, t.transform.translation.z = x, y, z
        t.transform.rotation.z = math.sin(yaw / 2)
        t.transform.rotation.w = math.cos(yaw / 2)
        return t

    def marker(self, frame, mid, kind, size, rgb, z=0.0):
        m = Marker()
        m.header.frame_id = frame
        m.header.stamp = self.get_clock().now().to_msg()
        m.ns = 'demo'
        m.id = mid
        m.type = kind
        m.action = Marker.ADD
        m.pose.position.z = z
        m.pose.orientation.w = 1.0
        m.scale.x = m.scale.y = m.scale.z = size
        m.color.r, m.color.g, m.color.b = rgb
        m.color.a = 1.0
        return m

    def tick(self):
        self.t += 0.1
        yaw = 0.2 * self.t
        self.tf.publish(TFMessage(transforms=[
            self.transform('map', 'base_link', math.cos(yaw), math.sin(yaw), 0.0, yaw),
            self.transform('base_link', 'sensor', 0.0, 0.0, 0.8, 0.0)]))
        self.markers.publish(self.marker('base_link', 0, Marker.CUBE, 0.6, (1.0, 0.1, 0.1), 0.3))
        self.markers.publish(self.marker('map', 1, Marker.SPHERE, 0.4, (0.1, 0.9, 0.2), 0.2))


def main():
    rclpy.init()
    rclpy.spin(DemoPub())


if __name__ == '__main__':
    main()
