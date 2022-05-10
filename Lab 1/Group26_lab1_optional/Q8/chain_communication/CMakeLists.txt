cmake_minimum_required(VERSION 3.5)
project(chain_communication)

# Default to C99
if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 99)
endif()
# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(std_msgs REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rmw_implementation REQUIRED)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # uncomment the line when a copyright and license is not present in all source files
  #set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # uncomment the line when this package is not in a git repo
  #set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

function(custom_executable target)
  add_executable(${target}${target_suffix}
          src/${target}.cpp)

  ament_target_dependencies(${target}${target_suffix}
          "rclcpp"
          "std_msgs"
          "rt")

  install(TARGETS ${target}${target_suffix}
          DESTINATION lib/${PROJECT_NAME})

endfunction()

macro(targets)
  custom_executable(talker_A)
  custom_executable(listener_B)
  custom_executable(listener_C)
  custom_executable(listener_D)
  custom_executable(listener_E)
#    custom_executable(talker_interprocess_single)
#    custom_executable(listener_interprocess_single)

endmacro()

call_for_each_rmw_implementation(targets GENERATE_DEFAULT)

ament_package()
