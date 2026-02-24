#!/usr/bin/env python3
"""
@file burger_launch.py
@brief Launch file for joystick teleoperation and robot control nodes.
"""

from launch import LaunchDescription
from launch_ros.actions import Node
#from launch.actions import TimerAction
from ament_index_python.packages import get_package_share_directory
import os
from launch_ros.parameter_descriptions import ParameterValue
from launch.substitutions import  Command
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource



def generate_launch_description():

    package_name = 'robot_descriptor'
    pkg_share = get_package_share_directory(package_name)

    xacro_file = os.path.join(pkg_share, 'descriptor', 'robot.urdf.xacro')


    robot_description = ParameterValue(
        Command([
            'xacro ',
            xacro_file,
            ' ',
            'use_sim_camera:=',
            'false'
        ]),
        value_type=str
    )

    workspace_config_path = '/root/ws/config/params.yaml'

    ld = LaunchDescription()

    # -----------------------------------------------------------------------
    # Joystick driver
    ld.add_action(
        Node(
            package='joy',
            executable='joy_node',
            name='joy',
            parameters=[{
                'device_id': 0,
                'deadzone': 0.05,
                'autorepeat_rate': 0.0,
                'sticky_buttons': False,
                'coalesce_interval_ms': 1
            }],
            output='screen',
            respawn=True,
            respawn_delay=2.0
        )
    )


    # -----------------------------------------------------------------------
    # Teleop Twist from joystick
    ld.add_action(
        Node(
            package='teleop_twist_joy',
            executable='teleop_node',
            name='teleop_twist_joy',
            parameters=[{
                'require_enable_button': True,
                'enable_button': 5,
                'enable_turbo_button': -1,
                'axis_linear.x': 1,
                'axis_linear.y': 2,
                'axis_linear.z': 3,
                'scale_linear.x': 1.0,
                'scale_linear.y': 1.0,
                'scale_linear.z': 0.0,
                'axis_angular.yaw': 2,
                'scale_angular.yaw': 1.0,
                'publish_stamped_twist': False,
            }],
            remappings=[('/cmd_vel', '/joy/cmd_vel')],
            output='screen'
        )
    )

    # -----------------------------------------------------------------------
    # Joy service handler
    ld.add_action(
        Node(
            package='joy_service_node',
            executable='joy_service_node',
            name='joy_service_handler',
            parameters=[workspace_config_path],
            output='screen'
        )
    )

    # -----------------------------------------------------------------------
    # STM32 driver
    ld.add_action(
        Node(
            package='stm32_nucleo_f303re_driver',
            executable='stm_driver',
            name='stm_driver',
            parameters=[workspace_config_path],
            output='screen'
        )
    )

    # -----------------------------------------------------------------------
    # LiDAR driver
    ld.add_action(
        Node(
            package='lidar_driver',
            executable='lidar_driver',
            name='lidar_driver',
            parameters=[workspace_config_path],
            output='screen'
        )
    )

    # -----------------------------------------------------------------------
    # Velocity reference from joystick
    ld.add_action(
        Node(
            package='joy_teleop_twist_ref',
            executable='joy_vel_ref',
            name='joy_vel_ref',
            parameters=[workspace_config_path],
            output='screen'
        )
    )

    # -----------------------------------------------------------------------
    # Robot state publisher

    ld.add_action(
        Node(
            package='robot_state_publisher',
            executable='robot_state_publisher',
            parameters=[{'robot_description': robot_description}],
            output='screen'
        )
    )
    ld.add_action(
       Node(
            package='joint_state_publisher',
            executable='joint_state_publisher'
        )
    )

# -----------------------------------------------------------------------
    # RealSense Camera (Depth Only + Low Res Color)
    realsense_node = Node(
        package='realsense2_camera',
        executable='realsense2_camera_node',
        name='realsense',
        namespace='burger',
        parameters=[{
            'enable_color': False,
            'enable_depth': True,
            'depth_module.depth_profile': '640x480x30',
            'align_depth.enable': False,
            'pointcloud.enable': False,
            'enable_infra': False,
            'enable_infra1': False,
            'enable_infra2': False,
            'pointcloud.allow_no_texture_points': False,
            'enable_gyro': False,
            'enable_accel': False,
            'decimation_filter.enable': True,
        }],
        output='screen'
    )

    ld.add_action(realsense_node)

    return ld
