import numpy as np
from rosbags.highlevel import AnyReader
from rosbags.typesys import Stores, get_typestore
from pathlib import Path

# ================= CONFIG =================
BAG_PATH = Path('/root/ws/rosbag2_2026_03_30-19_08_52')
TOPIC = '/imu'

G = 9.80665 

# ================= LOAD BAG =================
typestore = get_typestore(Stores.ROS2_HUMBLE)

acc_data = []
gyro_data = []

with AnyReader([BAG_PATH], default_typestore=typestore) as reader:
    connections = [c for c in reader.connections if c.topic == TOPIC]

    for connection, timestamp, rawdata in reader.messages(connections):
        msg = reader.deserialize(rawdata, connection.msgtype)

        acc = msg.linear_acceleration
        gyro = msg.angular_velocity

        acc_data.append([acc.x, acc.y, acc.z])
        gyro_data.append([gyro.x, gyro.y, gyro.z])

acc_data = np.array(acc_data)
gyro_data = np.array(gyro_data)

acc_data = acc_data * G

print(f"Samples: {len(acc_data)}")

# ================= BIAS =================
acc_bias = np.mean(acc_data, axis=0)
gyro_bias = np.mean(gyro_data, axis=0)

# ================= REMOVE BIAS =================
acc_zero = acc_data - acc_bias
gyro_zero = gyro_data - gyro_bias

# ================= COVARIANCE =================
acc_cov = np.cov(acc_zero.T)
gyro_cov = np.cov(gyro_zero.T)

# ================= STD DEV =================
acc_std = np.std(acc_zero, axis=0)
gyro_std = np.std(gyro_zero, axis=0)

# ================= PRINT =================
print("\n===== ACCELEROMETER =====")
print("Bias [m/s^2]:", acc_bias)
print("Std [m/s^2]:", acc_std)
print("Covariance:\n", acc_cov)

print("\n===== GYROSCOPE =====")
print("Bias [rad/s]:", gyro_bias)
print("Std [rad/s]:", gyro_std)
print("Covariance:\n", gyro_cov)

# ================= FORMAT FOR ROS =================
def cov_to_ros(cov):
    return [
        float(cov[0,0]), float(cov[0,1]), float(cov[0,2]),
        float(cov[1,0]), float(cov[1,1]), float(cov[1,2]),
        float(cov[2,0]), float(cov[2,1]), float(cov[2,2]),
    ]


print("\nlinear_acceleration_covariance:")
print(cov_to_ros(acc_cov))

print("\nangular_velocity_covariance:")
print(cov_to_ros(gyro_cov))