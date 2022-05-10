#!/bin/bash
source /opt/ros/dashing/setup.bash
colcon build
source install/setup.bash
ros2 run petrinet node --listen_rate=0 --talk_rate=10 --number=0 --listeners="1,0.5" --listeners="2,0.5" &
ros2 run petrinet node --number=1 --listen_rate=6 --queue_max=5 &
ros2 run petrinet node --number=2 --listen_rate=6 --queue_max=5 &
