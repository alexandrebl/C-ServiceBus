#include <iostream>

//UID Generator
#include "tools.h"
//Asio hanlder manager
#include "asiohandler.h"

//Main method
int main(int argc, const char* argv[])
{
    //Correlation ID
    const std::string correlation(uuid());

    //Service
    boost::asio::io_service ioService;
    //Initialize service handler
    AsioHandler handler(ioService);
    //Initialize connect event
    handler.connect("127.0.0.1", 5672);

    //Connect to service bus
    AMQP::Connection connection(&handler, AMQP::Login("guest", "guest"), "/");

    //Channel
    AMQP::Channel channel(&connection);
    
    //Callback event
    AMQP::QueueCallback callback = [&](const std::string &name, int msgcount, int consumercount)
    {
        //AMQP Message
        AMQP::Envelope env("30");
        //Correlation identifier
        env.setCorrelationID(correlation);
        //Reply queue
        env.setReplyTo(name);        
        //Publish message
        channel.publish("","rpc_queue",env);
        //Console message
        std::cout<<" [x] Requesting fib(30)"<<std::endl;
    };
    
    //Declare queue
    channel.declareQueue(AMQP::exclusive).onSuccess(callback);

    //Timer to timeout
    boost::asio::deadline_timer t(ioService, boost::posix_time::millisec(100));
    
    //Callback message
    auto receiveCallback = [&](const AMQP::Message &message, 
            uint64_t deliveryTag, bool redelivered)
    {
        //Verify correlation identifier
        if(message.correlationID() != correlation) return;

        //Console message
        std::cout<<" [.] Got "<<message.message()<<std::endl;
        
        //Initialize timer
        t.async_wait([&](const boost::system::error_code&){ioService.stop();});
    };

    //Consumer
    channel.consume("", AMQP::noack).onReceived(receiveCallback);

    //Start service
    ioService.run();
    
    //Return
    return 0;
}