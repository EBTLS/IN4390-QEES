#include "rclcpp/rclcpp.hpp"
#include "rclcpp/parameter.hpp"
#include "std_msgs/msg/string.hpp"

#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include <stdio.h>
#include <thread>

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

int init_num_int;

std::string s;

class talker_pingpong : public rclcpp::Node {
public :

//    constructor
    talker_pingpong();

    static int talker_number;
//    recorder of receive time

//    actions for publisher
    void pub_action();

    int message_pub(std::string message_filename, std::string output_filename);

//    actions for subscriber
    std::string read_datafile(std::string message_filename);

private:

    int node_id;

    int publisher_count;    // this->publisher_count is current evaluation number (< EVAL_NUM)
    int subscriber_count;

    int msg_type;

    FILE *fp_pub= nullptr;
    FILE *fp_ack= nullptr;

    struct timespec tp_pub;
    struct timespec tp_ack;

    std::string bytedata;

    double publish_time[EVAL_NUM];
    double subscribe_time[EVAL_NUM];


    std::string ack_filename = "./evaluation/ack_time/ack_time_256byte.txt";

    void call_back_record_message(const std_msgs::msg::String::SharedPtr msg);

//    QoS of this node
    rmw_qos_profile_t talker_qos_profile;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr talker_publisher;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr talker_subscription;


};

int talker_pingpong::talker_number = 0;

talker_pingpong::talker_pingpong() : Node("talker_pingpong") {

    talker_number++;
    this->publisher_count=-1;
    this->subscriber_count=-1;
    this->node_id = talker_number;
    this->msg_type=0;
//    this->talker_qos_profile=qos_profile;

    this->declare_parameter("QoS_Policy");
    int QoS_Policy_param;
    this->get_parameter_or("QoS_Policy", QoS_Policy_param, QoS_Policy);

    if (QoS_Policy_param == 1) {
        this->talker_qos_profile = rmw_qos_profile_reliable;
    } else if (QoS_Policy_param == 2) {
        this->talker_qos_profile = rmw_qos_profile_best_effort;
    } else if (QoS_Policy_param == 3) {
        this->talker_qos_profile = rmw_qos_profile_history;
    }

    talker_publisher = this->create_publisher<std_msgs::msg::String>("pingpong_test", this->talker_qos_profile);
    talker_subscription = this->create_subscription<std_msgs::msg::String>("pingpong_ack",
                                                                           std::bind(
                                                                                   &talker_pingpong::call_back_record_message,
                                                                                   this, std::placeholders::_1),
                                                                                   this->talker_qos_profile);
//   ::shared_ptr<talker_pingpong> talker_node;
//    talker_node.reset(this);


}

int talker_pingpong::message_pub(std::string message_filename, std::string output_filename) {
    if (-1 < this->publisher_count) {

//        printf("prepare msg");
        auto msg = std::make_shared<std_msgs::msg::String>();
        std::stringstream ss;
        // Create message
        ss << this->publisher_count;
        s = ss.str() + bytedata;
        msg->data = s;

        // Time recording
        if (clock_gettime(CLOCK_REALTIME, &this->tp_pub) < 0) {
            perror("clock_gettime begin");
            return 0;
        }
        this->publish_time[this->publisher_count] = (double) this->tp_pub.tv_sec + (double) this->tp_pub.tv_nsec / (double) 1000000000L;

//        printf("prepare_publish");

        this->talker_publisher->publish(msg); // publish message
    } else if (this->publisher_count == -1) {
        this->bytedata = this->read_datafile(message_filename.c_str());
    }

// Output this->publish_time [] to publish_time_ * byte.txt after evaluation
    if (this->publisher_count == EVAL_NUM - 1) {
        if ((this->fp_pub = fopen(output_filename.c_str(), "w")) != NULL) {
            for (int i = 0; i < EVAL_NUM; i++) {

                if (fprintf(this->fp_pub, "%18.9lf\n", this->publish_time[i]) < 0) {
                    // Write error
                    printf("error : can't output publish_time.txt %f",this->publish_time[i]);
                    break;
                }
            }
            fclose(this->fp_pub);
        } else {
            printf("error : can't output publish_time.txt ");
        }
        this->publisher_count = -2; // initilize for next date size
    }


    this->publisher_count++;
    return 0;
}


void talker_pingpong::pub_action() {

//    auto chatter_pub = node->create_publisher<std_msgs::msg::String>("chatter", custom_qos_profile);

    rclcpp::WallRate loop_rate(PUBLISH_Hz);


    printf("start evaluation 256byte \n");

    while (rclcpp::ok()) {
        message_pub("./evaluation/byte_data/data_256byte.txt",
                    "./evaluation/publish_time/publish_time_256byte.txt");
        if (this->publisher_count == -1) {
            printf("end this data size evaluation \n");
            break;
        }

//        rclcpp::spin_some(shared_from_this());
        loop_rate.sleep();
    }

//    rclcpp::spin_some(shared_from_this());
    usleep(5000000);

    printf("start evaluation 512byte \n");

    while (rclcpp::ok()) {
        message_pub("./evaluation/byte_data/data_512byte.txt",
                    "./evaluation/publish_time/publish_time_512byte.txt");
        if (this->publisher_count == -1) {
            printf("end this data size evaluation \n");
            break;
        }

//        rclcpp::spin_some(shared_from_this());
        loop_rate.sleep();
    }

//    rclcpp::spin_some(shared_from_this());
    usleep(5000000);

    printf("start evaluation 1Kbyte \n");

    while (rclcpp::ok()) {
        message_pub("./evaluation/byte_data/data_1Kbyte.txt",
                    "./evaluation/publish_time/publish_time_1Kbyte.txt");
        if (this->publisher_count == -1) {
            printf("end this data size evaluation \n");
            break;
        }

//        rclcpp::spin_some(shared_from_this());
        loop_rate.sleep();
    }

//    rclcpp::spin_some(shared_from_this());
    usleep(5000000);

    printf("start evaluation 2Kbyte \n");

    while (rclcpp::ok()) {
        message_pub("./evaluation/byte_data/data_2Kbyte.txt",
                    "./evaluation/publish_time/publish_time_2Kbyte.txt");
        if (this->publisher_count == -1) {
            printf("end this data size evaluation \n");
            break;
        }

//        rclcpp::spin_some(shared_from_this());
        loop_rate.sleep();
    }

//    rclcpp::spin_some(shared_from_this());
    usleep(5000000);

    printf("start evaluation 4Kbyte \n");

    while (rclcpp::ok()) {
        message_pub("./evaluation/byte_data/data_4Kbyte.txt",
                    "./evaluation/publish_time/publish_time_4Kbyte.txt");
        if (this->publisher_count == -1) {
            printf("end this data size evaluation \n");
            break;
        }

//        rclcpp::spin_some(shared_from_this());
        loop_rate.sleep();
    }

//    rclcpp::spin_some(shared_from_this());
    usleep(5000000);

    printf("start evaluation 8Kbyte \n");

    while (rclcpp::ok()) {
        message_pub("./evaluation/byte_data/data_8Kbyte.txt",
                    "./evaluation/publish_time/publish_time_8Kbyte.txt");
        if (this->publisher_count == -1) {
            printf("end this data size evaluation \n");
            break;
        }

//        rclcpp::spin_some(shared_from_this());
        loop_rate.sleep();
    }

//    rclcpp::spin_some(shared_from_this());
    usleep(5000000);

    printf("start evaluation 16Kbyte \n");

    while (rclcpp::ok()) {
        message_pub("./evaluation/byte_data/data_16Kbyte.txt",
                    "./evaluation/publish_time/publish_time_16Kbyte.txt");
        if (this->publisher_count == -1) {
            printf("end this data size evaluation \n");
            break;
        }

//        rclcpp::spin_some(shared_from_this());
        loop_rate.sleep();
    }

//    rclcpp::spin_some(shared_from_this());
    usleep(5000000);

    printf("start evaluation 32Kbyte \n");

    while (rclcpp::ok()) {
        message_pub("./evaluation/byte_data/data_32Kbyte.txt",
                    "./evaluation/publish_time/publish_time_32Kbyte.txt");
        if (this->publisher_count == -1) {
            printf("end this data size evaluation \n");
            break;
        }

//        rclcpp::spin_some(shared_from_this());
        loop_rate.sleep();
    }

//    rclcpp::spin_some(shared_from_this());
    usleep(5000000);

    printf("start evaluation 64Kbyte \n");

    while (rclcpp::ok()) {
        message_pub("./evaluation/byte_data/data_64Kbyte.txt",
                    "./evaluation/publish_time/publish_time_64Kbyte.txt");
        if (this->publisher_count == -1) {
            printf("end this data size evaluation \n");
            break;
        }

//        rclcpp::spin_some(shared_from_this());
        loop_rate.sleep();
    }

//    rclcpp::spin_some(shared_from_this());
    usleep(5000000);

    printf("start evaluation 128Kbyte \n");
    while (rclcpp::ok()) {
        message_pub("./evaluation/byte_data/data_128Kbyte.txt",
                    "./evaluation/publish_time/publish_time_128Kbyte.txt");
        if (this->publisher_count == -1) {
            printf("end this data size evaluation \n");
            break;
        }
//        rclcpp::spin_some(shared_from_this());
        loop_rate.sleep();
    }

//    rclcpp::spin_some(shared_from_this());

    printf("start evaluation 256Kbyte \n");
    while (rclcpp::ok()) {
        message_pub("./evaluation/byte_data/data_256Kbyte.txt",
                    "./evaluation/publish_time/publish_time_256Kbyte.txt");
        if (this->publisher_count == -1) {
            printf("end this data size evaluation \n");
            break;
        }
        loop_rate.sleep();
    }

    usleep(5000000);

    printf("start evaluation 512Kbyte \n");
    while (rclcpp::ok()) {
        message_pub("./evaluation/byte_data/data_512Kbyte.txt",
                    "./evaluation/publish_time/publish_time_512Kbyte.txt");
        if (this->publisher_count == -1) {
            printf("end this data size evaluation \n");
            break;
        }
//        rclcpp::spin_some(shared_from_this());
        loop_rate.sleep();
    }

    usleep(5000000);

    printf("start evaluation 1Mbyte \n");
    while (rclcpp::ok()) {
        message_pub("./evaluation/byte_data/data_1Mbyte.txt",
                    "./evaluation/publish_time/publish_time_1Mbyte.txt");
        if (this->publisher_count == -1) {
            printf("end this data size evaluation \n");
            break;
        }
//        rclcpp::spin_some(shared_from_this());
        loop_rate.sleep();
    }

    usleep(5000000);

    printf("start evaluation 2Mbyte \n");
    while (rclcpp::ok()) {
        message_pub("./evaluation/byte_data/data_2Mbyte.txt",
                    "./evaluation/publish_time/publish_time_2Mbyte.txt");
        if (this->publisher_count == -1) {
            printf("end this data size evaluation \n");
            break;
        }
//        rclcpp::spin_some(shared_from_this());
        loop_rate.sleep();
    }

    usleep(10000000);

    printf("start evaluation 4Mbyte \n");
    while (rclcpp::ok()) {
        message_pub("./evaluation/byte_data/data_4Mbyte.txt",
                    "./evaluation/publish_time/publish_time_4Mbyte.txt");
        if (this->publisher_count == -1) {
            printf("end this data size evaluation \n");
            break;
        }
//        rclcpp::spin_some(shared_from_this());
        loop_rate.sleep();
    }

    // followthrough transactions
    this->publisher_count = 0;
    while (rclcpp::ok()) {
        auto msg = std::make_shared<std_msgs::msg::String>();
        std::stringstream ss;
        ss << "end" << this->publisher_count;
        msg->data = ss.str();
        this->talker_publisher->publish(msg);
        if (this->publisher_count++ == 100) {
            printf("---end evaluation---\n");
            break;
        }
//        rclcpp::spin_some(shared_from_this());
        loop_rate.sleep();
    }

}


// A function that takes a file name as an argument and returns the contents of the file
std::string talker_pingpong::read_datafile(std::string message_filename) {

    // Read data from data_ * byte.txt to std :: string bytedata
    std::ifstream ifs(message_filename.c_str());
    if (ifs.fail()) {
        std::cerr << "data_*byte.txt do not exist.\n";
        exit(0);
    }

    std::string bytedata;
    getline(ifs, bytedata);

    return bytedata;
}

//call back function for subscriber
void talker_pingpong::call_back_record_message(const std_msgs::msg::String::SharedPtr msg) {

    if (clock_gettime(CLOCK_REALTIME, &this->tp_ack) < 0) {
        perror("clock_gettime begin");
    }
    double receive_time = (double) this->tp_ack.tv_sec + (double) this->tp_ack.tv_nsec / (double) 1000000000L;
//    printf("receive_message");

    std::string termination = msg->data.c_str();
    if (termination.find("end") != std::string::npos) {
        printf("---end evaluation---\n");
        rclcpp::shutdown();
    }

    if (this->subscriber_count == -1) {

        printf("message: %s \n",msg->data.c_str());
        // Initialize this->subscriber_count
        char init_num_char = *(msg->data.c_str());;
        char *init_num_pt = &init_num_char;
//        this->msg_type = atoi(init_num_pt);
//        printf("msg_type: %d \n", this->msg_type);
        this->subscriber_count = atoi(init_num_pt + 1);
        printf("subscriber_count: %d \n ", this->subscriber_count);
//        printf("current_time %f \n",receive_time);
        init_num_int = this->subscriber_count;    // if init_num_int is not 0, some messages are lost.

        // printf("first recieved number: %d \n\n", count);
        printf("message loss : %d \n", init_num_int);
        printf("current_message_type: %d \n", this->msg_type);
//        printf("eval_loop %d \n", this->msg_type);
    }

    // evaluation
    if (this->subscriber_count < EVAL_NUM - 1) {
        this->subscribe_time[this->subscriber_count] = receive_time;
//        printf("current_time %f \n",this->subscribe_time[this->subscriber_count]);
        this->subscriber_count++;
    } else if (this->subscriber_count == EVAL_NUM - 1) {
        this->subscribe_time[this->subscriber_count] = receive_time;

        // Output this->subscribe_time[] collectively to subscribe_tim_*bytee.txt after evaluation
        if ((this->fp_ack = fopen(this->ack_filename.c_str(), "w")) != NULL) {

            // write init_num
//            printf("output_file: %s",this->ack_filename);
//            std::cout<<this->ack_filename<<std::endl;
            if (fprintf(this->fp_ack, "%d\n", init_num_int) < 0) {
                // Write error
                printf("error : can't output ack_time_*byte.txt'");
            }

            // write this->subscribe_time[]
            for (int i = 0; i < EVAL_NUM; i++) {
                if (fprintf(this->fp_ack, "%18.9lf\n", this->subscribe_time[i]) < 0) {
                    // Write error
                    printf("error : can't output ack_time_*byte.txt'");
                    break;
                }
            }

            // printf("output data %d \n", this->msg_type);

            fclose(this->fp_ack);
        } else {
            printf("error : can't output ack_time_*byte.txt'");
        }

        // Initialization of evaluation
        this->subscriber_count = -1;                    // initilize for next date size

        // this->msg_type=11;				// update for next data size

        this->msg_type++;

        if (this->msg_type == 1) {
            this->ack_filename = "./evaluation/ack_time/ack_time_512byte.txt";
        } else if (this->msg_type == 2) {
            this->ack_filename = "./evaluation/ack_time/ack_time_1Kbyte.txt";
        } else if (this->msg_type == 3) {
            this->ack_filename = "./evaluation/ack_time/ack_time_2Kbyte.txt";
        } else if (this->msg_type == 4) {
            this->ack_filename = "./evaluation/ack_time/ack_time_4Kbyte.txt";
        } else if (this->msg_type == 5) {
            this->ack_filename = "./evaluation/ack_time/ack_time_8Kbyte.txt";
        } else if (this->msg_type == 6) {
            this->ack_filename = "./evaluation/ack_time/ack_time_16Kbyte.txt";
        } else if (this->msg_type == 7) {
            this->ack_filename = "./evaluation/ack_time/ack_time_32Kbyte.txt";
        } else if (this->msg_type == 8) {
            this->ack_filename = "./evaluation/ack_time/ack_time_64Kbyte.txt";
        } else if (this->msg_type == 9) {
            this->ack_filename = "./evaluation/ack_time/ack_time_128Kbyte.txt";
        } else if (this->msg_type == 10) {
            this->ack_filename = "./evaluation/ack_time/ack_time_256Kbyte.txt";
        } else if (this->msg_type == 11) {
            this->ack_filename = "./evaluation/ack_time/ack_time_512Kbyte.txt";
        } else if (this->msg_type == 12) {
            this->ack_filename = "./evaluation/ack_time/ack_time_1Mbyte.txt";
        } else if (this->msg_type == 13) {
            this->ack_filename = "./evaluation/ack_time/ack_time_2Mbyte.txt";
        } else if (this->msg_type == 14) {
            this->ack_filename = "./evaluation/ack_time/ack_time_4Mbyte.txt";
        } else if (this->msg_type == 15) {
            //   End of measurement
            this->subscriber_count = EVAL_NUM;
        }

    }

//    rclcpp::spin_some(shared_from_this());
//    rclcpp::spin(shared_from_this());
}

void spin_caller(rclcpp::Node::SharedPtr node_ptr){
    rclcpp::spin(node_ptr);

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

    auto node = std::make_shared<talker_pingpong>();

//    int ret=pthread_crea)

//    t.detach();

    std::thread t(spin_caller,node);
    t.detach();

//    rclcpp::spin(node);

    node->pub_action();


//    printf("start evaluation 256byte \n");

#pragma GCC diagnostic pop

    return 0;
}