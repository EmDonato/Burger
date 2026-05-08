#!/usr/bin/env python3
"""
@file burger_launch.py
@brief Launch file for joystick teleoperation and robot control nodes.
"""

from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import TimerAction
from ament_index_python.packages import get_package_share_directory
import os
from launch_ros.parameter_descriptions import ParameterValue
from launch.substitutions import  Command



def generate_launch_description():

    workspace_config_path = '/home/ema/Desktop/Burger/config/params.yaml'

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

    return ld
