#!/usr/bin/env python3
"""
@file burger_launch.py
@brief Launch file for joystick teleoperation and robot control nodes.

"""
import os

from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import ExecuteProcess
from launch.actions import LogInfo
import rclpy.logging

def generate_launch_description():
    workspace_config_path = '/root/ws/config/params.yaml'
    logger = rclpy.logging.get_logger('burger_launcher')        
        
    device_id = -1

    if os.path.exists("/dev/input/js1"):
        device_id = 1
        logger.info(" Joystick in /dev/input/js1")
    elif os.path.exists("/dev/input/js0"):
        device_id = 0
        logger.info(" Joystick in /dev/input/js0")
    else:
        logger.error("JOYSTICK NOT FOUND!")

    """
    Generate and return the ROS2 launch description.

    @returns LaunchDescription containing all configured actions.
    """
    ld = LaunchDescription()

    # -----------------------------------------------------------------------
    # Joystick input: publishes sensor_msgs/Joy messages
    # Parameters:
    #   device_id             ID of joystick device (/dev/input/jsX)
    #   deadzone              Minimum axis movement to register
    #   autorepeat_rate       Rate at which buttons repeat when held
    #   sticky_buttons        If True, buttons latch until read
    #   coalesce_interval_ms  Time window to combine events
    if device_id >= 0 :
        ld.add_action(Node(
            package='joy',
            executable='joy_node',
            name='joy',
            parameters=[{
                'device_id': device_id,
                'deadzone': 0.05,
                'autorepeat_rate': 0.0,
                'sticky_buttons': False,
                'respawn' : True,           
                'coalesce_interval_ms': 1
            }],
            output='screen'
        ))

    # -----------------------------------------------------------------------
    # Teleoperation: map joystick axes/buttons to geometry_msgs/Twist
    # Parameters:
    #   require_enable_button  Must hold enable_button to send commands
    #   enable_button          Button index to enable teleop
    #   enable_turbo_button    Button index for turbo mode (-1=disabled)
    #   axis_linear.*, axis_angular.*  Axis indices for motion
    #   scale_linear.*, scale_angular.* Scaling factors for speed
    #   publish_stamped_twist  If True, publish TwistStamped
    ld.add_action(Node(
        package='teleop_twist_joy',
        executable='teleop_node',
        name='teleop_twist_joy',
        parameters=[{
            'require_enable_button': True,
            'enable_button': 10,
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
    ))
    ld.add_action(Node(
            package='joy_service_node',
            executable='joy_service_node',
            name='joy_service_handler',
            parameters=[workspace_config_path],
            output='screen'
        ))
    ld.add_action(Node(
        package='stm32_nucleo_f303re_driver',
        executable='stm_driver',
        name='stm_driver',
        parameters=[workspace_config_path],
        output='screen'
    ))

    ld.add_action(Node(
        package='joy_teleop_twist_ref',
        executable='joy_vel_ref',
        name='joy_vel_ref',
        parameters=[workspace_config_path],
        output='screen'
    ))


    return ld