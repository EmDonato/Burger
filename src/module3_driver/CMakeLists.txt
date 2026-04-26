cmake_minimum_required(VERSION 3.8)
project(module3_driver)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ===== DEPENDENCIES =====
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(OpenCV REQUIRED)

# ===== EXECUTABLE =====
add_executable(module3 src/module3.cpp)

target_include_directories(module3 PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

target_compile_features(module3 PUBLIC cxx_std_17)

# ===== LINK =====
ament_target_dependencies(
  module3
  rclcpp
  sensor_msgs
)

target_link_libraries(module3 ${OpenCV_LIBS})

# ===== INSTALL (IMPORTANTE) =====
install(TARGETS module3
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

# ===== EXPORT =====
ament_package()