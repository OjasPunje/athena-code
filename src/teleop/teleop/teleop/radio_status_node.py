#!/usr/bin/env python3

from typing import Optional

import rclpy
from rclpy.duration import Duration
from rclpy.node import Node
from rclpy.qos import DurabilityPolicy, QoSProfile
from rosidl_runtime_py.utilities import get_message
from std_msgs.msg import Bool


class RadioStatusNode(Node):
    def __init__(self) -> None:
        super().__init__("radio_status")

        self.declare_parameter("input_topic", "/rear_ackermann_controller/reference")
        self.declare_parameter("input_type", "geometry_msgs/msg/TwistStamped")
        self.declare_parameter("output_topic", "/radio/status")
        self.declare_parameter("timeout_sec", 1.0)
        self.declare_parameter("publish_rate_hz", 5.0)
        self.declare_parameter("latch_output", True)

        self.input_topic = self.get_parameter("input_topic").get_parameter_value().string_value
        self.input_type = self.get_parameter("input_type").get_parameter_value().string_value
        self.output_topic = self.get_parameter("output_topic").get_parameter_value().string_value
        self.timeout_sec = self.get_parameter("timeout_sec").get_parameter_value().double_value
        self.publish_rate_hz = self.get_parameter("publish_rate_hz").get_parameter_value().double_value
        self.latch_output = self.get_parameter("latch_output").get_parameter_value().bool_value

        if self.timeout_sec <= 0.0:
            raise ValueError("timeout_sec must be > 0")
        if self.publish_rate_hz <= 0.0:
            raise ValueError("publish_rate_hz must be > 0")

        msg_type = get_message(self.input_type)

        pub_qos = QoSProfile(depth=1)
        if self.latch_output:
            pub_qos.durability = DurabilityPolicy.TRANSIENT_LOCAL

        self.publisher_ = self.create_publisher(Bool, self.output_topic, pub_qos)
        self.subscription_ = self.create_subscription(
            msg_type,
            self.input_topic,
            self.message_callback,
            10,
        )

        self.last_seen_time: Optional[rclpy.time.Time] = None
        self.last_status: Optional[bool] = None
        timer_period = 1.0 / self.publish_rate_hz
        self.timer = self.create_timer(timer_period, self.publish_status)

        self.get_logger().info(
            f"Monitoring '{self.input_topic}' ({self.input_type}) and publishing "
            f"link status on '{self.output_topic}' with {self.timeout_sec:.2f}s timeout"
        )

    def message_callback(self, _msg) -> None:
        self.last_seen_time = self.get_clock().now()

    def publish_status(self) -> None:
        now = self.get_clock().now()
        connected = False

        if self.last_seen_time is not None:
            connected = (now - self.last_seen_time) <= Duration(seconds=self.timeout_sec)

        if self.last_status is None or connected != self.last_status:
            state = "connected" if connected else "disconnected"
            self.get_logger().info(f"Radio link marked {state}")
            self.last_status = connected

        msg = Bool()
        msg.data = connected
        self.publisher_.publish(msg)


def main(args=None) -> None:
    rclpy.init(args=args)
    node = RadioStatusNode()
    try:
        rclpy.spin(node)
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == "__main__":
    main()
