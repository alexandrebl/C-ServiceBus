#include <iostream>

#include "asiohandler.h"

int main(void)
{
    boost::asio::io_service ioService;
    AsioHandler handler(ioService);
    
    int const limit = 1000;
    
    std::cout << "Application End!'" << std::endl;
    
    handler.connect("127.0.0.1", 5672);

    AMQP::Connection connection(&handler, AMQP::Login("guest", "guest"), "/");
    AMQP::Channel channel(&connection);

    boost::asio::deadline_timer t(ioService, boost::posix_time::millisec(1000));
    
    channel.onReady([&]()
    {
        if(handler.connected())
        {
            channel.declareQueue("FirstQueue");
            
            std::cout << "Initializing to sent " << limit << " messages" << std::endl;
            
            for(int x = 0; x < 1000; x++){
                channel.publish("", "FirstQueue", "Hello World!");
            }
            
            std::cout << "Messages has been processed" << std::endl;

            t.async_wait([&](const boost::system::error_code&){ioService.stop();});
            
            std::cout << "Application End!'" << std::endl;
        }
    });

    ioService.run();
    return 0;
}