# Time reference for logging and identification
std_msgs/Header header

# PWM values applied to the wheels
# Convention:
# pwm[0] -> left wheel
# pwm[1] -> right wheel
float32[] speed
int32[] pwm
