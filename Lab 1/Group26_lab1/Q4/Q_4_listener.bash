
#!/bin/bash

#source ./install/setup.bash

#cd ./build
#ros2 run interprocess_eval listener_interprocess_single__rmw_connext_cpp 

array_name=("fastrtps" "connext" "opensplice")

# for dds_type in ${array_name[*]}
# do

dds_type="fastrtps"

echo $dds_type

source ./install/setup.bash
# cd ./build
# echo $dds_type

# gnome-terminal -e "source ./install/setup.bash" & "ros2 run interprocess_eval talker_interprocess_single__rmw_${dds_type}_cpp"

ros2 run interprocess_eval listener_interprocess__rmw_${dds_type}_cpp




