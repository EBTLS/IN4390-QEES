
#!/bin/bash

#source ./install/setup.bash

#cd ./build
#ros2 run interprocess_eval listener_interprocess_single__rmw_connext_cpp 

array_name=("fastrtps" "connext" "opensplice")

# for dds_type in ${array_name[*]}
# do

dds_type="fastrtps"

echo $dds_type

result_256="transport_time_256byte"
result_128="transport_time_128Kbyte"


source ./install/setup.bash
# cd ./build
# echo $dds_type

# gnome-terminal -e "source ./install/setup.bash" & "ros2 run interprocess_eval talker_interprocess_single__rmw_${dds_type}_cpp"

ros2 run interprocess_eval talker_interprocess__rmw_${dds_type}_cpp

./calculate

from_path="/home/jxzhang/Desktop/TUDelft/Q2/QEES/lab1/evaluation/transport_time/"
target_path="/home/jxzhang/Desktop/TUDelft/Q2/QEES/lab1/Q_4_Data/${dds_type}/"
from_file=`ls ${from_path}`
target_file=`ls ${target_path}`

# rename file
history_number=0
for filename in $target_file
do
    echo $filename
    # if $filename == "transport_time_256byte.txt"
    # then
        history_number=$(($history_number+1))
    # fi
done
# history_number=`expr $history_number/2`
echo $history_number

from_file_name="${from_path}${result_256}.txt"
target_file_name="${target_path}${result_256}_${history_number}.txt"

# copy file
cp ${from_file_name} ${target_file_name}
# for filename in $from_file
# do
#     cp ${from_path}${result_256}".txt" ${target_path}${filename}${history_number}".txt"
# done

#clean result

# rename file

from_file_name="${from_path}${result_128}.txt"
target_file_name="${target_path}${result_128}_${history_number}.txt"

# copy file
cp ${from_file_name} ${target_file_name}

source ./clean_evaltime.bash

cd ..
cd ..


# done




