import os
from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory

def generate_launch_description():
    config_file = os.path.join(get_package_share_directory('umdloop-can'), 'config', 'can_config.yaml')
    return LaunchDescription([
        Node(
            package='umdloop-can',
            executable='can_node',
            name='can_node',
            output='screen',
            parameters=[config_file]
        )
    ])
