FROM ros:humble

ENV LANG=C.UTF-8
ENV LC_ALL=C.UTF-8
ENV FASTRTPS_DEFAULT_PROFILES_FILE=/opt/ros/humble/share/ros2/ros2_default_profile.xml
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    python3-pip \
    python3-colcon-common-extensions \
    \
    ros-humble-joy \
    ros-humble-teleop-twist-joy \
    ros-humble-teleop-twist-keyboard \
    ros-humble-rmw-fastrtps-cpp \
    ros-humble-joy-teleop \
    \
    ros-humble-diagnostic-updater \
    ros-humble-image-transport \
    ros-humble-cv-bridge \
    ros-humble-camera-info-manager \
    \
    libopencv-dev \
    libboost-dev \
    python3-opencv \
    \
    nano \
    iputils-ping \
    udev \
    pkg-config \
    libusb-1.0-0-dev \
    libglfw3-dev \
    libssl-dev \
    libgtk-3-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    \
    && mkdir -p /opt/ros/humble/share/ros2/ \
    && touch /opt/ros/humble/share/ros2/ros2_default_profile.xml \
    && rm -rf /var/lib/apt/lists/*

RUN echo "source /opt/ros/humble/setup.bash" >> ~/.bashrc

# ===============================
# librealsense (system library)
# ===============================
WORKDIR /opt

RUN git clone https://github.com/IntelRealSense/librealsense.git && \
    cd librealsense && \
    mkdir build && cd build && \
    cmake .. \
      -DBUILD_EXAMPLES=OFF \
      -DBUILD_GRAPHICAL_EXAMPLES=OFF \
      -DBUILD_WITH_TM2=OFF \
      -DFORCE_RSUSB_BACKEND=ON \
      -DCMAKE_BUILD_TYPE=Release && \
    make -j$(nproc) && \
    make install && \
    ldconfig

# ===============================
# Workspace (mounted by devcontainer)
# ===============================
WORKDIR /root/ws

ENTRYPOINT ["/ros_entrypoint.sh"]
CMD ["bash"]
