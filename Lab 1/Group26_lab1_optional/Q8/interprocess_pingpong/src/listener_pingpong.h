#include "rclcpp/rclcpp.hpp"
#include "rclcpp/parameter.hpp"
#include "std_msgs/msg/string.hpp"

#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include <stdio.h>

#include <time.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sched.h>


#define EVAL_NUM 900    // evaluation number for each data size
#define PUBLISH_Hz 10
#define QoS_Policy 3    // 1 means "reliable", 2 means "best effort", 3 means "history"
#define RUN_REAL_TIME

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

static const rmw_qos_profile_t rmw_qos_profile_reliable = {
        RMW_QOS_POLICY_HISTORY_KEEP_ALL,
        100,
        RMW_QOS_POLICY_RELIABILITY_RELIABLE,
        RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL
};

static const rmw_qos_profile_t rmw_qos_profile_best_effort = {
        RMW_QOS_POLICY_HISTORY_KEEP_LAST,
        1,
        RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT,
        RMW_QOS_POLICY_DURABILITY_VOLATILE
};

static const rmw_qos_profile_t rmw_qos_profile_history = {
        RMW_QOS_POLICY_HISTORY_KEEP_LAST,
        100,                            // depth option for HISTORY
        RMW_QOS_POLICY_RELIABILITY_RELIABLE,
        RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL
};

struct timespec tp1;
int i, count = -1;    // count is current evaluation number (< EVAL_NUM)
double publish_time[EVAL_NUM];

std::string s, bytedata;
FILE *fp;

class listener_pingpong : public rclcpp::Node {
public :

//    constructor
    listener_pingpong();

    static int listener_number;
//    recorder of receive time
    double  subscribe_time[EVAL_NUM];


private:

    int node_id;

    std::string output_filename = "./evaluation/ack_time/ack_time_256byte.txt";

    void call_back_record_message(const std_msgs::msg::String::SharedPtr msg);

//    QoS of this node
    rmw_qos_profile_t listener_qos_profile;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr listener_publisher;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr listener_subscription;


};

int listener_pingpong::listener_number = 0;

listener_pingpong::listener_pingpong() : Node("listener_pingpong") {

    listener_number++;
    this->node_id = listener_number;
//    this->listener_qos_profile=qos_profile;

    this->declare_parameter("QoS_Policy");
    int QoS_Policy_param;
    this->get_parameter_or("QoS_Policy",QoS_Policy_param,QoS_Policy);

    if (QoS_Policy_param == 1) {
        this->listener_qos_profile= rmw_qos_profile_reliable;
    } else if (QoS_Policy_param == 2) {
        this->listener_qos_profile = rmw_qos_profile_best_effort;
    } else if (QoS_Policy_param == 3) {
        this->listener_qos_profile = rmw_qos_profile_history;
    }

    listener_publisher = this->create_publisher<std_msgs::msg::String>("pingpong_ack",this->listener_qos_profile);
    listener_subscription = this->create_subscription<std_msgs::msg::String>("pingpong_test", 10,
                                                                           std::bind(
                                                                                   &listener_pingpong::call_back_record_message,
                                                                                   this, std::placeholders::_1));

}

//call back function for subscriber
void listener_pingpong::call_back_record_message(const std_msgs::msg::String::SharedPtr msg) {

    if (clock_gettime(CLOCK_REALTIME,&tp1) < 0) {
        perror("clock_gettime begin");
    }
    double receive_time = (double)tp1.tv_sec + (double)tp1.tv_nsec/ (double)1000000000L;
    printf("receive_message");

    std::string termination = msg->data.c_str();
    if (termination.find("end") != std::string::npos) {
        printf("---end evaluation---\n");
        rclcpp::shutdown();
    }

    auto ack = std::make_shared<std_msgs::msg::String>();
    std::stringstream ss;
    // Create message
    ss << ;
    s = ss.str() + bytedata;
    msg->data = s;

    int init_num_int;

    int msg_type;

    if (count == -1) {

        // Initialize count
        char init_num_char = *( msg->data.c_str());
        msg_type=atoi(&init_num_char+1);
        char *init_num_pt = &init_num_char;
        count = atoi(init_num_pt);
        init_num_int = count;	// if init_num_int is not 0, some messages are lost.

        // printf("first recieved number: %d \n\n", count);
        printf("message loss : %d \n", init_num_int);
        printf("current_message_type: d% \n",msg_type);
        printf("eval_loop %d \n", msg_type);
    }

    // evaluation
    if (count < EVAL_NUM - 1) {
        subscribe_time[count] = receive_time;
        count++;
    }
    else if (count == EVAL_NUM - 1) {
        subscribe_time[count] = receive_time;

        // Output subscribe_time[] collectively to subscribe_tim_*bytee.txt after evaluation
        if ((fp = fopen(output_filename.c_str(), "w")) != NULL) {

            // write init_num
            if (fprintf(fp, "%d\n",init_num_int ) < 0) {
                // Write error
                printf("error : can't output ack_time_*byte.txt'");
            }

            // write subscribe_time[]
            for (i = 0; i < EVAL_NUM; i++) {
                if (fprintf(fp, "%18.9lf\n", subscribe_time[i]) < 0) {
                    // Write error
                    printf("error : can't output ack_time_*byte.txt'");
                    break;
                }
            }

            // printf("output data %d \n", msg_type);

            fclose(fp);
        }
        else {
            printf("error : can't output ack_time_*byte.txt'");
        }

        // Initialization of evaluation
        count = -1;					// initilize for next date size



        // msg_type=11;				// update for next data size

        if( msg_type == 1){
            output_filename = "./evaluation/ack_time/ack_time_512byte.txt";
        }
        else if( msg_type == 2){
            output_filename = "./evaluation/ack_time/ack_time_1Kbyte.txt";
        }else if( msg_type == 3){
            output_filename = "./evaluation/ack_time/ack_time_2Kbyte.txt";
        }else if( msg_type == 4){
            output_filename = "./evaluation/ack_time/ack_time_4Kbyte.txt";
        }else if( msg_type == 5){
            output_filename = "./evaluation/ack_time/ack_time_8Kbyte.txt";
        }else if( msg_type == 6){
            output_filename = "./evaluation/ack_time/ack_time_16Kbyte.txt";
        }else if( msg_type == 7){
            output_filename = "./evaluation/ack_time/ack_time_32Kbyte.txt";
        }else if( msg_type == 8){
            output_filename = "./evaluation/ack_time/ack_time_64Kbyte.txt";
        }else if( msg_type == 9){
            output_filename = "./evaluation/ack_time/ack_time_128Kbyte.txt";
        }else if( msg_type == 10){
            output_filename = "./evaluation/ack_time/ack_time_256Kbyte.txt";
        }
        else if( msg_type == 11){
            output_filename = "./evaluation/ack_time/ack_time_512Kbyte.txt";
        }
        else if( msg_type == 12){
            output_filename = "./evaluation/ack_time/ack_time_1Mbyte.txt";
        }else if( msg_type == 13){
            output_filename = "./evaluation/ack_time/ack_time_2Mbyte.txt";
        }else if( msg_type == 14){
            output_filename = "./evaluation/ack_time/ack_time_4Mbyte.txt";
        }else if( msg_type == 15){
            //   End of measurement
            count = EVAL_NUM;
        }

    }

//    rclcpp::spin_some(shared_from_this());
//    rclcpp::spin(shared_from_this());
}


int main(int argc, char *argv[]) {
    mlockall(MCL_FUTURE);        // lock all cached memory into RAM and prevent future dynamic memory allocations

    usleep(1000);

#ifdef RUN_REAL_TIME
    sched_param pri = {94};
    if (sched_setscheduler(0, SCHED_FIFO, &pri) == -1) { // set FIFO scheduler
        perror("sched_setattr");
        exit(EXIT_FAILURE);
    }
#endif

    rclcpp::init(argc, argv);

    auto node=std::make_shared<listener_pingpong>();

    rclcpp::spin(node);

//    node->pub_action();


//    printf("start evaluation 256byte \n");

#pragma GCC diagnostic pop

    return 0;
}