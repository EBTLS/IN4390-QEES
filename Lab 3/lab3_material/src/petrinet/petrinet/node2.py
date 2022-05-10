import rclpy
import os
import random
import argparse
from enum import Enum
from rclpy.node import Node

from std_msgs.msg import String, UInt8

TIME_STEP = 0.5
class State(Enum):
    ACTIVE = 0
    INACTIVE = 1

class QeesNode2(Node):

    def __init__(self, listeners, number=0, queue_max=1000, queue_start=100):
        super().__init__('qees_node_%d' % number)

        self.number = number            # node number
        self.state = State.ACTIVE       # node state
        self.queue = queue_start        # Token queue
        self.queue_max = queue_max      # maximum number of tokens in all queues

        # Topic where stop command will be sent
        self.stop_subscription = self.create_subscription(String, 'stop', self.stop_callback, 10)

        # Topic where tokens are received for this node
        self.token_subscription = self.create_subscription(
            String,
            'topic/node%d' % number,
            self.token_received_callback,
            10)

        # Topic where all node state changes are communicated
        self.state_publisher = self.create_publisher(UInt8, 'state', 10)

        # Topic where other node state changes will be handled
        self.state_subscription = self.create_subscription(
            UInt8,
            'state',
            self.state_callback,
            10)

        # Topic where token levels are published
        self.token_publisher = self.create_publisher(String, 'token', 10)
        self.token_publish_timer = self.create_timer(0.25, self.publish_token_count)

      
        # Nodes where tokens will be sent to
        self.nodes = [int(l[0]) for l in listeners]

        # Rate of token generation for each node
        self.rates = {int(l[0]): float(l[1]) for l in listeners}

        self.rate_sum = sum(self.rates)

        # Create publishers for nodes which this node will be sending tokens to
        self.node_publishers = {i: self.create_publisher(String, 'topic/node%d' % i, 10) for i in self.nodes}

        self.outgoing_token_timers = {}

        print(listeners)

        for l in listeners:
            node = int(l[0])
            rate = float(l[1])

            if rate <= 0:
                raise ValueError("Rate must be strictly positive")

            self.get_logger().info('Node %d will send tokens to node %d with rate %d/s' % (self.number, node, rate))

        self.outgoing_token_timers = self.create_timer(TIME_STEP, self.node_token_generation_callback)

        self.get_logger().info('Launched QEES Node %d' % number)

    def publish_token_count(self):
        """Publish the current token count to a topic
        """

        msg = String()
        msg.data = "Node %d has %04d tokens" % (self.number, self.queue)
        self.token_publisher.publish(msg)

    def token_received_callback(self, msg):
        """Callback called when a token message is received.

        Parameters: 
        msg: The received message, which has a format of "<SOURCE NODE NUMBER>:<NUMBER OF INCOMING TOKENS> 
        """
        source, data = msg.data.split(":")
        source = int(source)
        data = float(data)
        if self.state == State.ACTIVE:
            self.queue += data
            self.get_logger().info('Node %d got "%d" from "%d" - [Queue=%s]' % (self.number, data, source, self.queue))

        if self.queue > self.queue_max:
            self.deactivate()

    def deactivate(self):
        """Deactivate a node
        """

        self.state = State.INACTIVE

        # Publish state to all other nodes
        msg = UInt8()
        msg.data = self.number
        self.state_publisher.publish(msg)

    def node_token_generation_callback(self):
        """Callback called by token generation timer.

        Sends tokens to other nodes. Message is sent using format <CURRENT_NODE_NUMBER>:<NUMBER_OF_TOKENS>
        to the /topic/node<NODE_NUMBER> topic.

        This message is then handled in the token_received_callback
        """

        outgoing = {node: self.queue*rate*TIME_STEP*0.01 for node, rate in self.rates.items()}
        for node, rate in outgoing.items():
            if self.state == State.ACTIVE and self.queue > rate and rate > 0:
                msg = String()
                data = rate
                msg.data = "%d:%12.12f" % (self.number, data)
                if node in self.node_publishers:
                    self.node_publishers[node].publish(msg)
                    self.queue -= rate
                    self.get_logger().info('Publishing: "%4.3f" to node %d [Queue=%d]' % (data, node, self.queue))
                else:
                    self.get_logger().info("Could not find node %s in %s" % (node, self.node_publishers))

    def state_callback(self, msg):
        """Callback called when a state change message is received.
        """
        # Set the node weight to 0 
        node = msg.data
        self.rates[node] = 0

    def stop_callback(self, msg):
        """Callback called when message is received on the 'stop' topic.
        """

        # Dirty way of shutting node down forcefully
        raise ChildProcessError("Exiting")

def main(args=None):

    # Parse name
    parser = argparse.ArgumentParser()
    parser.add_argument('--number', help='The node number')
    parser.add_argument('--queue_max', help='The queue max', default=1000)
    parser.add_argument('--queue_start', help='The starting value of the queue', default=100)
    parser.add_argument('--listeners', action='append', help='The listeners. Format = <LISTENER_NODE_NUMBER>,<LISTENER_OUTPUT_RATE>')

    cl_args = parser.parse_args()  
    rclpy.init(args=args)

    if cl_args.listeners is None:
        cl_args.listeners = []

    listeners = list(map(lambda s: s.split(","), cl_args.listeners))

    qees_subscriber = QeesNode2(
        number=int(cl_args.number),
        queue_max=int(cl_args.queue_max),
        queue_start=int(cl_args.queue_start),
        listeners=listeners)

    rclpy.spin(qees_subscriber)

    # Destroy the node explicitly
    qees_subscriber.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()

