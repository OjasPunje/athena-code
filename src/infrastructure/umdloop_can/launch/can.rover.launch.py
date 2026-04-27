from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='umdloop_can',
            executable='can_node',
        )
    ])
