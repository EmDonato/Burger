FROM ros:humble

ENV LANG=C.UTF-8
ENV LC_ALL=C.UTF-8
ENV DEBIAN_FRONTEND=noninteractive

ENV RMW_IMPLEMENTATION=rmw_cyclonedds_cpp

# ==========================================
# Base system + ROS dependencies
# ==========================================

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    curl \
    nano \
    pkg-config \
    udev \
    python3-pip \
    python3-colcon-common-extensions \
    iputils-ping \
    alsa-utils \
    \
    libusb-1.0-0-dev \
    libssl-dev \
    libgtk-3-dev \
    libglfw3-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    \
    libopencv-dev \
    libboost-dev \
    python3-opencv \
    \
    libgstreamer1.0-0 \
    libgstreamer-plugins-base1.0-0 \
    \
    ros-humble-joy \
    ros-humble-joy-teleop \
    ros-humble-teleop-twist-joy \
    ros-humble-teleop-twist-keyboard \
    \
    ros-humble-rmw-fastrtps-cpp \
    ros-humble-rmw-cyclonedds-cpp \
    \
    ros-humble-plotjuggler-ros \
    ros-humble-rqt-gui \
    ros-humble-rqt-plot \
    \
    ros-humble-xacro \
    ros-humble-tf2-tools \
    ros-humble-diagnostic-updater \
    \
    ros-humble-image-transport \
    ros-humble-compressed-image-transport \
    ros-humble-compressed-depth-image-transport \
    ros-humble-cv-bridge \
    ros-humble-camera-info-manager \
    \
    ros-humble-robot-state-publisher \
    ros-humble-joint-state-publisher \
    \
    ros-humble-rosbag2-py \
    ros-humble-rosbag2-storage-mcap \
    \
    && rm -rf /var/lib/apt/lists/*


# ==========================================
# Python packages
# ==========================================

RUN pip3 install --no-cache-dir \
    numpy \
    pandas \
    matplotlib \
    scipy \
    rosbags \
    spatialmath-python \
    jupyterlab \
    piper-tts


# ==========================================
# Piper voices
# ==========================================

RUN mkdir -p /voices

WORKDIR /voices

RUN wget -O amy.onnx \
    https://huggingface.co/rhasspy/piper-voices/resolve/main/en/en_US/amy/medium/en_US-amy-medium.onnx

RUN wget -O amy.onnx.json \
    https://huggingface.co/rhasspy/piper-voices/resolve/main/en/en_US/amy/medium/en_US-amy-medium.onnx.json


# ==========================================
# ROS environment
# ==========================================

RUN echo "source /opt/ros/humble/setup.bash" >> /root/.bashrc


# ==========================================
# librealsense
# ==========================================

WORKDIR /opt

RUN git clone https://github.com/IntelRealSense/librealsense.git && \
    cd librealsense && \
    mkdir build && cd build && \
    cmake .. \
        -DBUILD_EXAMPLES=OFF \
        -DBUILD_GRAPHICAL_EXAMPLES=OFF \
        -DBUILD_WITH_TM2=OFF \
        -DCMAKE_BUILD_TYPE=Release && \
    make -j$(nproc) && \
    make install && \
    ldconfig


# ==========================================
# ROS2 workspace
# ==========================================

WORKDIR /root/ws/src

RUN git clone https://github.com/realsenseai/realsense-ros.git && \
    cd realsense-ros && \
    git checkout ros2-master


# ==========================================
# Build workspace
# ==========================================

WORKDIR /root/ws

RUN /bin/bash -c "source /opt/ros/humble/setup.bash && \
    colcon build --cmake-args -DCMAKE_BUILD_TYPE=Release"


RUN echo "source /root/ws/install/setup.bash" >> /root/.bashrc


# ==========================================
# Default
# ==========================================

ENTRYPOINT ["/ros_entrypoint.sh"]

CMD ["bash"]