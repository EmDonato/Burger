from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription, SetEnvironmentVariable
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, Command
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue
from ament_index_python.packages import get_package_share_directory
import os


def generate_launch_description():

    package_name = 'robot_descriptor'
    pkg_share = get_package_share_directory(package_name)

    world_file = os.path.join(pkg_share, 'worlds', 'Nutt.world')
    xacro_file = os.path.join(pkg_share, 'descriptor', 'robot.urdf.xacro')
    bridge_yaml = os.path.join(pkg_share, 'config', 'gz_bridge.yaml')
    config_joy = os.path.join(pkg_share, 'config', 'config_sim.yaml')

    use_sim_camera = LaunchConfiguration('use_sim_camera')

    robot_description = ParameterValue(
        Command([
            'xacro ',
            xacro_file,
            ' ',
            'use_sim_camera:=',
            use_sim_camera
        ]),
        value_type=str
    )

    resource_path = os.path.dirname(pkg_share)

    # Gazebo ufficiale launch
    gazebo = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(
                get_package_share_directory('ros_gz_sim'),
                'launch',
                'gz_sim.launch.py'
            )
        ),
        launch_arguments={
            'gz_args': f'-r {world_file}'
        }.items()
    )

    return LaunchDescription([

        DeclareLaunchArgument(
            'use_sim_camera',
            default_value='true'
        ),

        SetEnvironmentVariable(
            name='GZ_SIM_RESOURCE_PATH',
            value=os.path.join(resource_path)
        ),
        # Robot state publisher
        Node(
            package='robot_state_publisher',
            executable='robot_state_publisher',
            parameters=[{'robot_description': robot_description}],
            output='screen'
        ),
        gazebo,
        # Spawn robot
        Node(
            package='ros_gz_sim',
            executable='create',
            arguments=[
                '-topic', 'robot_description',
                '-name', 'burger',
                '-z', '0.5'
            ],
            output='screen'
        ),

        # Joint state publisher
        Node(
            package='joint_state_publisher',
            executable='joint_state_publisher'
        ),

        # Bridge
        Node(
            package='ros_gz_bridge',
            executable='parameter_bridge',
            arguments=[
                '--ros-args',
                '-p',
                f'config_file:={bridge_yaml}',
            ],
            output='screen'
        ),

        # RViz
        Node(
            package='rviz2',
            executable='rviz2'
        ),
        # -----------------------------------------------------------------------
        # Joystick
        Node(
            package='joy',
            executable='joy_node',
            name='joy',
            parameters=[{
                'device_id': 1,
                'deadzone': 0.05,
                'autorepeat_rate': 0.0,
                'sticky_buttons': False,
                'coalesce_interval_ms': 1
            }],
            output='screen',
            respawn=True,
            respawn_delay=2.0
        ),
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
        ),
        Node(
            package='joy_teleop_twist_ref',
            executable='joy_vel_ref',
            name='joy_vel_ref',
            parameters=[config_joy],
            output='screen'
        )
    ])