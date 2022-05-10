1. source first
2. make sure the CPU frequency scaling is off, by following the instructions in the Page 6(mainly the adding following conent)
3. run the listener node located in Page8, then the taker node
4. run calculate(an exctuable file of cpp), copy the result to someplace else, then run the clean_evaltime.bash to reset
experiment 1 is done

5. run the artificial node in a new terminal, then repeat 3, 4
experiment 2 is done

6. delete the #define RUN REAL TIME in the talker_interprocess.cpp and listener_interprocess.cpp
7. do step3, 4
experiment 3 is done

8. do step 5 then step 3,4
experiment 4 is done

make sure the CPU frequency scaling is on, by reseet the whole file which was modifed in step2(or just only delete"GOVERNOR")
repeat experiment 1-4, name them as experiment 5-8