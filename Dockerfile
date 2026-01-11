# Base ROS2 Humble (Ubuntu 22.04)
FROM ros:humble

# Locale
ENV LANG=C.UTF-8
ENV LC_ALL=C.UTF-8
ENV FASTRTPS_DEFAULT_PROFILES_FILE=/opt/ros/humble/share/ros2/ros2_default_profile.xml

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    python3-pip \
    python3-colcon-common-extensions \
    ros-humble-joy \
    ros-humble-teleop-twist-joy \
    ros-humble-teleop-twist-keyboard \
    ros-humble-rmw-fastrtps-cpp \
    ros-humble-joy-teleop \
    libopencv-dev \
    libboost-dev \
    python3-opencv \
    nano \
    iputils-ping \
    && mkdir -p /opt/ros/humble/share/ros2/ \
    && touch /opt/ros/humble/share/ros2/ros2_default_profile.xml \
    && rm -rf /var/lib/apt/lists/*

RUN echo "source /opt/ros/humble/setup.bash" >> ~/.bashrc
    
WORKDIR /root/ws

ENTRYPOINT ["/ros_entrypoint.sh"]
CMD ["bash"]