#!/bin/bash

set -e

echo "[BURGER] ===== Startup begin ====="

CONTAINER_NAME="burger_container"
IMAGE_NAME="vsc-burger-4742daa97fb39e6f564bf2e529b7c2b0ad4226b23c015641ffa8d8c0cf7b63d4"
WORKSPACE="/home/ema/Desktop/burger"

# --------------------------------------------------
# 1. Attesa Docker (robusto)
# --------------------------------------------------
echo "[BURGER] Waiting for Docker..."

for i in {1..10}; do
    if docker info > /dev/null 2>&1; then
        echo "[BURGER] Docker is ready"
        break
    fi
    echo "[BURGER] Docker not ready yet... ($i)"
    sleep 2
done

# Se docker non è pronto → errore
if ! docker info > /dev/null 2>&1; then
    echo "[ERROR] Docker not available"
    exit 1
fi

# --------------------------------------------------
# 2. Cleanup container vecchio (se esiste)
# --------------------------------------------------
if docker ps -a --format '{{.Names}}' | grep -q "^${CONTAINER_NAME}$"; then
    echo "[BURGER] Removing old container"
    docker rm -f ${CONTAINER_NAME} || true
fi

# --------------------------------------------------
# 3. Avvio container + ROS2 (PROCESSO PRINCIPALE)
# --------------------------------------------------
echo "[BURGER] Starting container"

exec docker run \
    --name ${CONTAINER_NAME} \
    --network host \
    --privileged \
    -v /dev:/dev \
    -v ${WORKSPACE}:/root/ws \
    -e FASTRTPS_DEFAULT_PROFILES_FILE=/opt/ros/humble/share/ros2/ros2_default_profile.xml \
    ${IMAGE_NAME} \
    bash -c "
        set -e
        echo '[CONTAINER] Sourcing ROS2...'
        source /opt/ros/humble/setup.bash

        echo '[CONTAINER] Sourcing workspace...'
        source /root/ws/install/setup.bash

        echo '[CONTAINER] Launching ROS2...'
        ros2 launch stm32_nucleo_f303re_driver becon_cheese_burger_launch.py
    "

# NON ARRIVA QUI (exec sostituisce il processo)