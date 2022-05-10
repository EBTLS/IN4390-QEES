import rclpy
import os
import random
import argparse
from enum import Enum
from rclpy.node import Node

from std_msgs.msg import String, UInt8

class State(Enum):
    ACTIVE = 0
    INACTIVE = 1

class QeesNode(Node):

    def __init__(self, listeners, number=0, rate=0.5, queue_max=10, queue_take={}, talk_rate=0):
        super().__init__('qees_node_%d' % number)

        self.number = number            # node number
        self.state = State.ACTIVE       # node state
        self.queue = {}                 # incoming token queues (1 for each node which sends to this node)
        self.queue_take = queue_take    # number of tokens which will be removed from the queue for each node
        self.queue_max = queue_max      # maximum number of tokens in all queues
        
        self.notified_flag={}   # flag for whether has comes to a near-full condition (1 for each node which sends to this node)

        # Topic where stop command will be sent
        self.stop_subscription = self.create_subscription(String, 'stop', self.stop_callback, 10)

        # Topic where tokens are received for this node
        self.token_subscription = self.create_subscription(
            String,
            'topic/node%d' % number,
            self.token_received_callback,
            10)

        # Topic where all node state changes are communicated
        self.state_publisher = self.create_publisher(String, 'state', 10)

        # Topic where other node state changes will be handled
        self.state_subscription = self.create_subscription(
            UInt8,
            'state',
            self.state_callback,
            10)

        # Create timer for token removal from incoming token queues
        if rate > 0:
            self.token_removal_timer = self.create_timer(1/rate, self.token_removal_callback)

        # Nodes where tokens will be sent to
        self.nodes = [int(l[0]) for l in listeners]

        # Create publishers for nodes which this node will be sending tokens to
        self.node_publishers = {i: self.create_publisher(String, 'topic/node%d' % i, 10) for i in self.nodes}

        # Probability distribution for token generation
        self.node_weights = {int(l[0]): float(l[1]) for l in listeners}
        
        self.node_weights_copy={int(l[0]): float(l[1]) for l in listeners}
        # Create timer for token generation for nodes which are listening to this node
        if talk_rate > 0:
            self.node_token_generation_timer = self.create_timer(1/talk_rate, self.node_token_generation_callback)

        self.get_logger().info('Launched QEES Node %d' % number)

    def token_received_callback(self, msg):
        """Callback called when a token message is received.

        Parameters: 
        msg: The received message, which has a format of "<SOURCE NODE NUMBER>:<NUMBER OF INCOMING TOKENS> 
        """
        source, data = map(int, msg.data.split(":"))
        if self.state == State.ACTIVE:
            self.queue.setdefault(source, 0)
            self.notified_flag.setdefault(source,0)
            self.queue[source] += data
            self.get_logger().info('Node %d got "%d" from "%d" - [Queue=%s]' % (self.number, data, source, self.queue))

        if self.queue[source]==self.queue_max:
            self.notified_flag[source]=1
            self.notify_near_full(source)
            self.get_logger().info("Node %d is nearly full for talker %d" % (self.number,source))
                    

        if self.queue[source] > self.queue_max:
            self.deactivate()
            
    def notify_near_full(self,source):
        """
        Parameters
        ----------
        source : the id of sender
            notify sender this node is near full
        
        the msg to notify other it is full is <current_node_number>:<source_number>:<1>

        Returns
        -------
        None.

        """
        # msg=UInt8()
        msg = String()
        msg.data="%d:%d:%d" % (self.number,source,1)
        
        self.state_publisher.publish(msg)
        
        
    def notify_reactivate(self,source):
        """
        NOTIFY SENDER IT IS READY AGAIN
        
        Parameters
        ----------
        source : the id of sender
        
        the msg to notify other it is full is <current_node_number>:<source_number>:<0>

        Returns
        -------
        None.

        """
        
        # msg=UInt8()
        msg = String()
        msg.data="%d:%d:%d" % (self.number,source,0)
        
        self.state_publisher.publish(msg)
        
        

    def deactivate(self):
        """
        NOTIFY  OTHERS IT IS DEACTIVATED
        
        the msg to notify other it is deactivated is <current_node_number>:<0>:<2>
        

        Returns
        -------
        None.

        """

        self.state = State.INACTIVE

        # Publish state to all other nodes
        # msg = UInt8()
        msg = String()
        msg.data="%d:%d:%d" % (self.number,0,2)
        self.state_publisher.publish(msg)


    def node_token_generation_callback(self):
        """Callback called by token generation timer.

        Sends tokens to other nodes. Message is sent using format <CURRENT_NODE_NUMBER>:<NUMBER_OF_TOKENS>
        to the /topic/node<NODE_NUMBER> topic.

        This message is then handled in the token_received_callback
        """
        
        if self.state == State.ACTIVE:
            msg = String()
            data = 1
            msg.data = "%d:%d" % (self.number, data)

            # pick a node randomly using probability distribution
            node = random.choices(list(self.node_weights_copy.keys()), weights=list(self.node_weights_copy.values()))[0]
            if self.node_weights[node] <= 0:
                return
            self.node_publishers[node].publish(msg)
            self.get_logger().info('Publishing: "%d" to node %d' % (data, node))

    def token_removal_callback(self):
        """Callback called by token removal timer.

        Removes tokens from the incoming queues.
        """

        if self.state == State.ACTIVE:
            for k in self.queue:
                self.queue_take.setdefault(k, 1)    # if tokens are received from a source not specified in the queue_take, assume 1 token needs to be removed
            
            if all(self.queue.get(k, 0) >= v for k, v in self.queue_take.items()): # check if there are enough tokenes
                for k, v in self.queue_take.items():
                    self.queue[k] -= v              # remove the specified number of tokens 
                    
                    if self.queue[k]<=self.queue_max-1 and self.notified_flag[k]==1:
                        self.notify_reactivate(int(k))
                        self.notified_flag[k]=0
                        self.get_logger().info('sender is reactivated')

                if len(self.queue) > 0:             # do not print if no tokens have ever been received
                    self.get_logger().info('Removed from queue: now [Queue=%s]' % (self.queue))
            else:
                self.get_logger().info('Could not remove from queue: %s' % (self.queue))
                
            

    def state_callback(self, msg):
        """Callback called when a state change message is received.
        """
        # Set the node weight to 0 
        
        listener,talker,state = map(int, msg.data.split(":"))
        if state==2:
            self.node_weights[listener]=0
        elif state==1 and self.number==talker:
            self.node_weights[listener]=0
        elif state==0 and self.number==talker:
            self.node_weights[listener]=self.node_weights_copy[listener]
        

    def stop_callback(self, msg):
        """Callback called when message is received on the 'stop' topic.
        """

        # Dirty way of shutting node down forcefully
        raise ChildProcessError("Exiting")

def main(args=None):

    # Parse name
    parser = argparse.ArgumentParser()
    parser.add_argument('--number', help='The node number')
    parser.add_argument('--listen_rate', help='The rate')
    parser.add_argument('--talk_rate', help='The rate', default=0.0)
    parser.add_argument('--queue_max', help='The queue max', default=10)
    parser.add_argument('--queue_take', action='append', help='The takers')
    parser.add_argument('--listeners', action='append', help='The listeners')

    cl_args = parser.parse_args()  
    rclpy.init(args=args)

    if cl_args.queue_take is None:
        cl_args.queue_take = []

    if cl_args.listeners is None:
        cl_args.listeners = []

    queue_take = {}

    for s in cl_args.queue_take:
        parsed = list(map(int, s.split(',')))
        queue_take[parsed[0]] = parsed[1]

    listeners = list(map(lambda s: s.split(","), cl_args.listeners))

    qees_subscriber = QeesNode(
        number=int(cl_args.number),
        rate=float(cl_args.listen_rate),
        talk_rate=float(cl_args.talk_rate),
        queue_max=int(cl_args.queue_max),
        queue_take=queue_take,
        listeners=listeners)

    rclpy.spin(qees_subscriber)

    # Destroy the node explicitly
    qees_subscriber.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()
