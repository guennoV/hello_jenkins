//
//  main.cpp
//  uavia-software-next
//
//

#include <boost/asio/signal_set.hpp>

#include <coreKit/Log/Logger.hpp>
#include <coreKit/Log/Log.hpp>
#include <coreKit/Service/Handler.hpp>
#include <coreKit/Utils/StackTrace.hpp>

static coreKit::Logger logger( { "coreKit", "SampleService"  } );

class MyService : public coreKit::ServiceBase {
    
public:
    
    MyService() {
        throw std::runtime_error("This service must be explicitly instantiated");
    }
    
    MyService(const std::string &param) {
        logger->info("Create 'MyService' with param [" + param + "].");
    }
    
    virtual ~MyService() {
        logger->info("Destruct 'MyService' .");
    }
    
    void startService() override {
        // throw std::runtime_error("This is an error message .");
        logger->info("Starting service [{}]", getName());
    }
    
    void stopService() noexcept(true) override {
        // throw std::runtime_error("This is an error message .");
        logger->info("Stopping service [{}]", getName());
    }
    
    static coreKit::Service::Info getInfo() {
        return { "My super service", "0.0.1", { /* No dependency */ }, 0 /* No options */ };
    }
    
};

int main() {
    
    // Add StackTrace ability
    
    coreKit::handleStackTrace();
    
    // Add log sink
    
    {
        spdlog::sink_ptr sink = std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>();
        coreKit::Log::subscribe(sink);
    }
    
    // Signal handling
    
    boost::asio::io_service ioService;
    boost::asio::signal_set signals(ioService, SIGINT, SIGTERM);
    
    // Start handler
    
    coreKit::Service::HandlerCallbacks callbacks;
    coreKit::Service::Handler handler(callbacks);
    
    handler.subscribe<MyService>("Parameter");
    handler.start();
    
    // Log handler status
    
    logger->info("{}", handler.getStatus().toString());
    
    // Wait for signal
    
    signals.async_wait([&handler](const boost::system::error_code&,
                                  int) {
        handler.stop();
    });
    
    ioService.run();
    
    // Exit
    
    return EXIT_SUCCESS;
}
