cmake_minimum_required(VERSION 3.8)
project(go2_odometry_nav)

# Default to C++17
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(unitree_go REQUIRED)

# Include directories
include_directories(
  include
)

# Add executable
add_executable(go2_odom_nav_node src/go2_odom_nav.cpp)

ament_target_dependencies(go2_odom_nav_node
  rclcpp
  nav_msgs
  tf2
  tf2_ros
  tf2_geometry_msgs
  unitree_go
)

# Install executable
install(TARGETS
  go2_odom_nav_node
  DESTINATION lib/${PROJECT_NAME}
)

# Install launch files, config, etc. (optional)
# install(DIRECTORY launch config
#   DESTINATION share/${PROJECT_NAME}
# )

ament_package()