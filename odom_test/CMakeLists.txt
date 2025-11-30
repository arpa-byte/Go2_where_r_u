cmake_minimum_required(VERSION 3.8)
project(odom_test)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2 REQUIRED)                 # <-- NEW
find_package(tf2_geometry_msgs REQUIRED)
find_package(unitree_go REQUIRED)

add_executable(odom_test_node src/odom_test_node.cpp)
ament_target_dependencies(odom_test_node
  rclcpp nav_msgs tf2_ros tf2_geometry_msgs unitree_go)

install(TARGETS
  odom_test_node
  DESTINATION lib/${PROJECT_NAME})

ament_package()