#!/bin/bash
source /opt/ros/dashing/setup.bash
colcon build
source install/setup.bash
ros2 run petrinet node2 --queue_start=200 --queue_max=250 --number=1 --listeners="3,3" --listeners="2,1" &
ros2 run petrinet node2 --queue_start=200 --queue_max=250 --number=2 --listeners="5,1" &
ros2 run petrinet node2 --queue_start=200 --queue_max=250 --number=3 --listeners="5,2" --listeners="4,2" &
ros2 run petrinet node2 --queue_start=200 --queue_max=250 --number=4 --listeners="2,3" &
ros2 run petrinet node2 --queue_start=200 --queue_max=250 --number=5 --listeners="1,3" &
