cmake_minimum_required(VERSION 3.8)
project(odom_bridge_mid)

# Find all necessary ROS2 packages
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)

# --- CORRECTED LINE ---
# Add our executable node, matching your file name "odom_bridge_node_mid.cpp".
# We'll also name the executable target "odom_bridge_node_mid" for clarity.
add_executable(odom_bridge_node_mid src/odom_bridge_node_mid.cpp)

# Link against all the required ROS2 libraries
ament_target_dependencies(odom_bridge_node_mid
  "rclcpp"
  "nav_msgs"
  "geometry_msgs"
  "tf2_ros"
  "tf2_geometry_msgs"
)

# Manually specify the include directories for the Unitree SDK AND its third-party DDS headers.
target_include_directories(odom_bridge_node_mid PUBLIC
  "/home/arpan/unitree2/unitree_sdk2/include"
  "/home/arpan/unitree2/unitree_sdk2/thirdparty/include/ddscxx"
  "/home/arpan/unitree2/unitree_sdk2/thirdparty/include"
)

# Manually link against the pre-compiled Unitree SDK libraries and its dependencies.
target_link_libraries(odom_bridge_node_mid
  "/home/arpan/unitree2/unitree_sdk2/lib/x86_64/libunitree_sdk2.a"
  "/home/arpan/unitree2/unitree_sdk2/thirdparty/lib/x86_64/libddscxx.so"
  "/home/arpan/unitree2/unitree_sdk2/thirdparty/lib/x86_64/libddsc.so"
  Threads::Threads
)

# Apply the compiler definition required by the SDK.
target_compile_definitions(odom_bridge_node_mid PRIVATE UNITREE_SDK_VERSION_2)

# --- CORRECTED LINE ---
# Install the "odom_bridge_node_mid" executable target
install(
  TARGETS odom_bridge_node_mid
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
