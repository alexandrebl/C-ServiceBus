#include <iostream>

//Event manager definition
#include "asiohandler.h"

//First method to be invoked
int main(void)
{
    //Service class
    boost::asio::io_service ioService;
    //Manage service messages
    AsioHandler handler(ioService);
    
    //Initialize handle with connection
    handler.connect("127.0.0.1", 5672);

    //Connect to service bus
    AMQP::Connection connection(&handler, AMQP::Login("guest", "guest"), "/");
    //Load channel manager
    AMQP::Channel channel(&connection);
    
    //Declare Queue if not exist
    channel.declareQueue("FirstQueue");
    
    //Consume service bus messages
    channel.consume("FirstQueue", AMQP::noack).onReceived(
            [](const AMQP::Message &message,
                       uint64_t deliveryTag,
                       bool redelivered)
            {
                //Print messages
                std::cout <<"Received "<<message.message() << std::endl;
            });

    std::cout << "Press CTRL+C any key to exit\n";

    //Run service
    ioService.run();
    
    //Return
    return 0;
}