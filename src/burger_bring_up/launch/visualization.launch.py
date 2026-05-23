from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import AnyLaunchDescriptionSource
from launch.substitutions import Command
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue
from ament_index_python.packages import get_package_share_directory
import os


def generate_launch_description():

    # ================= ROBOT DESCRIPTION =================
    package_name = 'robot_descriptor'
    pkg_share = get_package_share_directory(package_name)

    xacro_file = os.path.join(pkg_share, 'descriptor', 'robot.urdf.xacro')

    robot_description = ParameterValue(
        Command([
            'xacro ',
            xacro_file,
            ' ',
            'use_sim_camera:=True'
        ]),
        value_type=str
    )

    # ================= FOXGLOVE BRIDGE =================
    foxglove_bridge_launch = IncludeLaunchDescription(
        AnyLaunchDescriptionSource(
            os.path.join(
                get_package_share_directory('foxglove_bridge'),
                'launch',
                'foxglove_bridge_launch.xml'
            )
        ),
        launch_arguments={
            'port': '8774',
            'address': '0.0.0.0'
        }.items()
    )

    # ================= NODES =================
    return LaunchDescription([

        # Foxglove bridge
        #foxglove_bridge_launch,

        # Robot state publisher
        Node(
            package='robot_state_publisher',
            executable='robot_state_publisher',
            parameters=[{'robot_description': robot_description}],
            output='screen'
        ),

        # RGB decompression
        Node(
            package='image_decompressor',
            executable='decompress_node',
            output='screen'
        ),

        # Depth decompression
        Node(
            package='depth_decompressor',
            executable='depth_decompressor_node',
            output='screen'
        ),
        # LLM node
        #Node(
        #    package="ollama",
        #    executable="ollama_node",
        #    output="screen",
        #)
    ])