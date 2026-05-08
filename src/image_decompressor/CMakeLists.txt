cmake_minimum_required(VERSION 3.8)
project(image_decompressor)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(OpenCV REQUIRED COMPONENTS core imgcodecs imgproc)
find_package(PkgConfig REQUIRED)
pkg_check_modules(TURBOJPEG REQUIRED libturbojpeg)

add_executable(decompress_node src/decompress_node.cpp)

ament_target_dependencies(decompress_node
  rclcpp
  sensor_msgs
)

target_include_directories(decompress_node PRIVATE
  ${OpenCV_INCLUDE_DIRS}
  ${TURBOJPEG_INCLUDE_DIRS}
)

target_link_libraries(decompress_node
  ${OpenCV_LIBS}
  ${TURBOJPEG_LIBRARIES}
)

install(TARGETS decompress_node
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()