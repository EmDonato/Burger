FROM ros:humble

ENV LANG=C.UTF-8
ENV LC_ALL=C.UTF-8
ENV FASTRTPS_DEFAULT_PROFILES_FILE=
ENV DEBIAN_FRONTEND=noninteractive
ENV RMW_IMPLEMENTATION=rmw_cyclonedds_cpp
# ===============================
# Base dependencies
# ===============================
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    python3-pip \
    python3-colcon-common-extensions \
    pkg-config \
    udev \
    libusb-1.0-0-dev \
    libssl-dev \
    libgtk-3-dev \
    libglfw3-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    libturbojpeg0-dev \
    \
    # ROS packages
    ros-humble-joy \
    ros-humble-teleop-twist-joy \
    ros-humble-teleop-twist-keyboard \
    ros-humble-rmw-fastrtps-cpp \
    ros-humble-joy-teleop \
    ros-humble-plotjuggler-ros \
    ros-humble-rqt-gui \
    ros-humble-rviz2 \
    ros-humble-depth-image-proc \
    ros-humble-rqt-plot \
    ros-humble-xacro \
    ros-humble-rosbag2-storage-mcap \
    ros-humble-image-transport \
    ros-humble-compressed-image-transport \
    ros-humble-diagnostic-updater \
    ros-humble-image-transport \
    ros-humble-cv-bridge \
    ros-humble-camera-info-manager \
    ros-humble-robot-state-publisher \
    ros-humble-joint-state-publisher \
    ros-humble-rmw-cyclonedds-cpp \
    ros-humble-tf2-tools \
    ros-humble-foxglove-bridge\
    \
    libopencv-dev \
    libboost-dev \
    python3-opencv \
    nano \
    iputils-ping \
    && rm -rf /var/lib/apt/lists/*

# ===============================
# Python scientific stack
# ===============================
RUN pip3 install --no-cache-dir \
    numpy \
    pandas \
    matplotlib \
    seaborn \
    scipy \
    mcap-ros2-support \
    rosbags \
    spatialmath-python \
    jupyterlab \
    nicegui

RUN echo "source /opt/ros/humble/setup.bash" >> ~/.bashrc


WORKDIR /root/ws
RUN . /opt/ros/humble/setup.sh && \
    colcon build --cmake-args -DCMAKE_BUILD_TYPE=Release

RUN echo "source /root/ws/install/setup.bash" >> ~/.bashrc

ENTRYPOINT ["/ros_entrypoint.sh"]
CMD ["bash"]