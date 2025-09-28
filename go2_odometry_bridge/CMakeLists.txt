cmake_minimum_required(VERSION 3.8)
project(go2_odometry_bridge)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(unitree_go REQUIRED)

add_executable(go2_odometry_bridge_node src/go2_odom_bridge_node.cpp)
ament_target_dependencies(
  go2_odometry_bridge_node
  "rclcpp"
  "nav_msgs"
  "tf2_ros"
  "tf2_geometry_msgs"
  "geometry_msgs"
  "unitree_go"
)

install(TARGETS
  go2_odometry_bridge_node
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
