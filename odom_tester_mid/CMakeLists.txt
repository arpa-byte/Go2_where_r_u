cmake_minimum_required(VERSION 3.8)
project(odom_tester_mid)

# Find necessary ROS2 packages
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)

# Add our executable node
add_executable(odom_tester_node src/odom_tester_node.cpp)

# Link against the required ROS2 libraries
ament_target_dependencies(odom_tester_node "rclcpp")

# --- CRITICAL FIXES ADAPTED FROM YOUR WORKING PROJECT ---

# 1. Manually specify the include directories for the Unitree SDK AND its third-party DDS headers.
#    This is the direct fix for the "dds/dds.hpp not found" error.
target_include_directories(odom_tester_node PUBLIC
  "/home/arpan/unitree2/unitree_sdk2/include"
  "/home/arpan/unitree2/unitree_sdk2/thirdparty/include/ddscxx"
  "/home/arpan/unitree2/unitree_sdk2/thirdparty/include"
)

# 2. Manually link against the pre-compiled Unitree SDK libraries and its dependencies.
target_link_libraries(odom_tester_node
  # The main static library for the SDK
  "/home/arpan/unitree2/unitree_sdk2/lib/x86_64/libunitree_sdk2.a"
  # The required DDS shared libraries
  "/home/arpan/unitree2/unitree_sdk2/thirdparty/lib/x86_64/libddscxx.so"
  "/home/arpan/unitree2/unitree_sdk2/thirdparty/lib/x86_64/libddsc.so"
  # The threads library is also a dependency
  Threads::Threads
)

# 3. Apply the compiler definition required by the SDK. This is likely very important.
target_compile_definitions(odom_tester_node PRIVATE UNITREE_SDK_VERSION_2)

# --- END OF CRITICAL FIXES ---


# Install the executable so "ros2 run" can find it
install(
  TARGETS odom_tester_node
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
