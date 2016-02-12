#include <iostream>
#include <algorithm>
#include <thread>
#include <chrono>
#include <string>
#include <boost/lexical_cast.hpp>

//Event manager
#include "asiohandler.h"

//Namespace
using boost::lexical_cast;

//Main method
int main(void)
{
    //Servide
    boost::asio::io_service ioService;
    //Event manager
    AsioHandler handler(ioService);
    
    //Message counter
    static long count = 0;
    
    //Server
    handler.connect("localhost", 5672);
    //Connection
    AMQP::Connection connection(&handler, AMQP::Login("guest", "guest"), "/");
    //Channel
    AMQP::Channel channel(&connection);

    //Define output queue
    channel.declareQueue("rpcqueue_out");
    //Define input queue
    channel.declareQueue("rpcqueue_in");
    
    //Service bus consumer
    channel.consume("").onReceived([&channel](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered)
    {
        //Counter
        count++;
        
        //Received message
        const std::string body = message.message();
        
        //New value
        int value = boost::lexical_cast<int>(body) + 1;
        //New message
        std::string msg = boost::lexical_cast<std::string>(value);        
        //Console mesage
        std::cout << "[" << count << "] / receive: " << body << " / processed: " << msg << std::endl;
        
        //AMQP Message
        AMQP::Envelope env(msg);
        //Correlation identifier
        env.setCorrelationID(message.correlationID());

        //Publish message
        channel.publish("", message.replyTo(), env);
        //Send ack for received message
        channel.ack(deliveryTag);
    });

    //Console message
    std::cout << " [x] Awaiting RPC requests" << std::endl;

    //Start service
    ioService.run();
    
    //Return
    return 0;
}