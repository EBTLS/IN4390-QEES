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


#define EVAL_NUM 500    // evaluation number for each data size
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

int init_num_int;

std::string s, bytedata;
FILE *fp;

class listener_pingpong : public rclcpp::Node {
public :

//    constructor
    listener_pingpong();

    static int listener_number;
//    recorder of receive time
    double subscribe_time[EVAL_NUM];


private:

    int node_id;

    int eval_loop_count;

    std::string output_filename = "./evaluation/subscribe_time_D/subscribe_time_256byte.txt";

    void call_back_record_message(const std_msgs::msg::String::SharedPtr msg);

//    QoS of this node
    rmw_qos_profile_t listener_qos_profile;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr listener_publisher;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr listener_subscription;


};

int listener_pingpong::listener_number = 0;

listener_pingpong::listener_pingpong() : Node("listener_D") {

    listener_number++;
    this->node_id = listener_number;
    this->eval_loop_count = 0;
//    this->listener_qos_profile=qos_profile;

    this->declare_parameter("QoS_Policy");
    int QoS_Policy_param;
    this->get_parameter_or("QoS_Policy", QoS_Policy_param, QoS_Policy);

    if (QoS_Policy_param == 1) {
        this->listener_qos_profile = rmw_qos_profile_reliable;
    } else if (QoS_Policy_param == 2) {
        this->listener_qos_profile = rmw_qos_profile_best_effort;
    } else if (QoS_Policy_param == 3) {
        this->listener_qos_profile = rmw_qos_profile_history;
    }

    listener_publisher = this->create_publisher<std_msgs::msg::String>("D2E", this->listener_qos_profile);
    listener_subscription = this->create_subscription<std_msgs::msg::String>("C2D",
                                                                             std::bind(
                                                                                     &listener_pingpong::call_back_record_message,
                                                                                     this, std::placeholders::_1),
                                                                             this->listener_qos_profile);
}

//call back function for subscriber
void listener_pingpong::call_back_record_message(const std_msgs::msg::String::SharedPtr msg) {

    if (clock_gettime(CLOCK_REALTIME, &tp1) < 0) {
        perror("clock_gettime begin");
    }
    double receive_time = (double) tp1.tv_sec + (double) tp1.tv_nsec / (double) 1000000000L;
    this->listener_publisher->publish(msg);
//    printf("receive_message");

    std::string termination = msg->data.c_str();
    if (termination.find("end") != std::string::npos) {
        rclcpp::WallRate loop_rate(PUBLISH_Hz);
        printf("---end evaluation---\n");
        while (rclcpp::ok()) {
            this->listener_publisher->publish(msg);
            if (count++ == 100) {
//                printf("---end evaluation---\n");
                break;
            }
            loop_rate.sleep();
        }
        rclcpp::shutdown();
    }


//    printf("ack published");

//  printf("received");


    if (count == -1) {

        // Initialize count
        char init_num_char = *(msg->data.c_str());
        char *init_num_pt = &init_num_char;
        count = atoi(init_num_pt);
        init_num_int = count;    // if init_num_int is not 0, some messages are lost.

        // printf("first recieved number: %d \n\n", count);
        printf("message loss : %d \n", init_num_int);
        printf("eval_loop %d \n", eval_loop_count);
    }
//
//    auto ack = std::make_shared<std_msgs::msg::String>();
//    std::stringstream ss;
//    // Create message
////    ss << eval_loop_count;
//    ss <<count;
//    s = ss.str() + " ack";
////    printf("eval_loop_count %d",eval_loop_count);
//    ack->data = s;

    // evaluation
    if (count < EVAL_NUM - 1) {
        subscribe_time[count] = receive_time;
//        ss.clear();
        count++;
    } else if (count == EVAL_NUM - 1) {
        subscribe_time[count] = receive_time;
//        ss.clear();

        // Output subscribe_time[] collectively to subscribe_tim_*bytee.txt after evaluation
        if ((fp = fopen(output_filename.c_str(), "w")) != NULL) {

            // write init_num
            if (fprintf(fp, "%d\n", init_num_int) < 0) {
                // Write error
                printf("error : can't output subscribe_time_*byte.txt'");
            }

            // write subscribe_time[]
            for (i = 0; i < EVAL_NUM; i++) {
                if (fprintf(fp, "%18.9lf\n", subscribe_time[i]) < 0) {
                    // Write error
                    printf("error : can't output subscribe_time_*byte.txt'");
                    break;
                }
            }

            // printf("output data %d \n", eval_loop_count);

            fclose(fp);
        } else {
            printf("error : can't output subscribe_time_*byte.txt'");
        }

        // Initialization of evaluation
        count = -1;                    // initilize for next date size


        eval_loop_count++;                // update for next data size

        if (eval_loop_count == 1) {
            output_filename = "./evaluation/subscribe_time_D/subscribe_time_512byte.txt";
        } else if (eval_loop_count == 2) {
            output_filename = "./evaluation/subscribe_time_D/subscribe_time_1Kbyte.txt";
        } else if (eval_loop_count == 3) {
            output_filename = "./evaluation/subscribe_time_D/subscribe_time_2Kbyte.txt";
        } else if (eval_loop_count == 4) {
            output_filename = "./evaluation/subscribe_time_D/subscribe_time_4Kbyte.txt";
        } else if (eval_loop_count == 5) {
            output_filename = "./evaluation/subscribe_time_D/subscribe_time_8Kbyte.txt";
        } else if (eval_loop_count == 6) {
            output_filename = "./evaluation/subscribe_time_D/subscribe_time_16Kbyte.txt";
        } else if (eval_loop_count == 7) {
            output_filename = "./evaluation/subscribe_time_D/subscribe_time_32Kbyte.txt";
        } else if (eval_loop_count == 8) {
            output_filename = "./evaluation/subscribe_time_D/subscribe_time_64Kbyte.txt";
        } else if (eval_loop_count == 9) {
            output_filename = "./evaluation/subscribe_time_D/subscribe_time_128Kbyte.txt";
        } else if (eval_loop_count == 10) {
            output_filename = "./evaluation/subscribe_time_D/subscribe_time_256Kbyte.txt";
        } else if (eval_loop_count == 11) {
            output_filename = "./evaluation/subscribe_time_D/subscribe_time_512Kbyte.txt";
        } else if (eval_loop_count == 12) {
            output_filename = "./evaluation/subscribe_time_D/subscribe_time_1Mbyte.txt";
        } else if (eval_loop_count == 13) {
            output_filename = "./evaluation/subscribe_time_D/subscribe_time_2Mbyte.txt";
        } else if (eval_loop_count == 14) {
            output_filename = "./evaluation/subscribe_time_D/subscribe_time_4Mbyte.txt";
        } else if (eval_loop_count == 15) {
            //   End of measurement
//            rclcpp::shutdown();
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

    auto node = std::make_shared<listener_pingpong>();

    printf("start evaluation\n");

    rclcpp::spin(node);

//    node->pub_action();


//    printf("start evaluation 256byte \n");

#pragma GCC diagnostic pop

    return 0;
}