cmake_minimum_required(VERSION 3.8)
project(go2_control_bridge)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)

add_executable(control_bridge_node src/control_bridge_node.cpp)

ament_target_dependencies(control_bridge_node "rclcpp" "geometry_msgs")

target_include_directories(control_bridge_node PUBLIC
  "/home/arpan/unitree2/unitree_sdk2/include"
  "/home/arpan/unitree2/unitree_sdk2/thirdparty/include/ddscxx"
  "/home/arpan/unitree2/unitree_sdk2/thirdparty/include"
)

target_link_libraries(control_bridge_node
  "/home/arpan/unitree2/unitree_sdk2/lib/x86_64/libunitree_sdk2.a"
  "/home/arpan/unitree2/unitree_sdk2/thirdparty/lib/x86_64/libddscxx.so"
  "/home/arpan/unitree2/unitree_sdk2/thirdparty/lib/x86_64/libddsc.so"
  Threads::Threads
)

target_compile_definitions(control_bridge_node PRIVATE UNITREE_SDK_VERSION_2)

install(
  TARGETS control_bridge_node
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
