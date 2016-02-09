#include <iostream>
#include <string>

//Event manager definition
#include "asiohandler.h"

//First method to be invoked
int main(void)
{
    //Service class
    boost::asio::io_service ioService;
    //Manage service messages
    AsioHandler handler(ioService);
    //Total of messages to be sent
    int const limit = 1000;
    
    //Console message
    std::cout << "Application End!'" << std::endl;

    //Initialize handle with connection
    handler.connect("127.0.0.1", 5672);

    //Connect to service bus
    AMQP::Connection connection(&handler, AMQP::Login("guest", "guest"), "/");
    //Load channel manager
    AMQP::Channel channel(&connection);

    //Define timeout to abort
    boost::asio::deadline_timer t(ioService, boost::posix_time::millisec(1000));
    
    //Ready event
    channel.onReady([&]()
    {
        //Verify if is connected
        if(handler.connected())
        {
            //Declare Queue if not exist
            channel.declareQueue("FirstQueue");
            
            //Console message
            std::cout << "Initializing to sent " << limit << " messages" << std::endl;
            
            //Loop for 1000 messages
            for(int index = 0; index < 1000; index++){
                //define message
                std::string msg = "Message number: " + std::to_string(index+1);
                
                //Publish message
                channel.publish("", "FirstQueue", msg);
            }
            
            //Console message
            std::cout << "Messages has been processed" << std::endl;

            //Start timer to always wait for 1 second
            t.async_wait([&](const boost::system::error_code&){
                //Stop service
                ioService.stop();
            });
            
            //Console message
            std::cout << "Application End!'" << std::endl;
        }
    });

    //Start service
    ioService.run();
    
    //Return
    return 0;
}