cmake_minimum_required(VERSION 3.8)
project(odom_logger)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)

add_executable(odom_logger_node src/odom_logger_node.cpp)

ament_target_dependencies(odom_logger_node "rclcpp")

target_include_directories(odom_logger_node PUBLIC
  "/home/arpan/unitree2/unitree_sdk2/include"
  "/home/arpan/unitree2/unitree_sdk2/thirdparty/include/ddscxx"
  "/home/arpan/unitree2/unitree_sdk2/thirdparty/include"
)

target_link_libraries(odom_logger_node
  "/home/arpan/unitree2/unitree_sdk2/lib/x86_64/libunitree_sdk2.a"
  "/home/arpan/unitree2/unitree_sdk2/thirdparty/lib/x86_64/libddscxx.so"
  "/home/arpan/unitree2/unitree_sdk2/thirdparty/lib/x86_64/libddsc.so"
  Threads::Threads
)

target_compile_definitions(odom_logger_node PRIVATE UNITREE_SDK_VERSION_2)

install(
  TARGETS odom_logger_node
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
