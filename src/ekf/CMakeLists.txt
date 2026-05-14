cmake_minimum_required(VERSION 3.8)
project(ekf)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
  )
endif()

find_package(ament_cmake REQUIRED)

find_package(rclcpp REQUIRED)

find_package(sensor_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav_msgs REQUIRED)

find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)

find_package(Eigen3 REQUIRED)


add_executable(
  ekf_node
  src/ekf.cpp
)


ament_target_dependencies(
  ekf_node

  rclcpp

  sensor_msgs
  geometry_msgs
  nav_msgs

  tf2
  tf2_ros
)


target_link_libraries(
  ekf_node
  Eigen3::Eigen
)

install(
  TARGETS ekf_node
  DESTINATION lib/${PROJECT_NAME}
)


if(BUILD_TESTING)

  find_package(ament_lint_auto REQUIRED)

  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)

  ament_lint_auto_find_test_dependencies()

endif()

ament_package()