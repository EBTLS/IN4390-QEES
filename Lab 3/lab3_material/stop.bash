#!/bin/bash
source /opt/ros/dashing/setup.bash
ros2 topic pub --once stop std_msgs/msg/String
