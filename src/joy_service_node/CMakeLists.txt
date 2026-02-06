cmake_minimum_required(VERSION 3.8)
project(joy_service_node)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(stm32_nucleo_f303re_driver REQUIRED)

add_executable(joy_service_node
  src/joy_service_node.cpp
)

ament_target_dependencies(joy_service_node
  rclcpp
  sensor_msgs
  stm32_nucleo_f303re_driver
)

install(TARGETS joy_service_node
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()