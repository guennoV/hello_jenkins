//
//  main.cpp
//  uavia-software-next
//
//

#include <boost/asio/signal_set.hpp>

#include <coreKit/App/App.hpp>
#include <coreKit/Log/Log.hpp>
#include <coreKit/Utils/StackTrace.hpp>

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
    
    // Create app
    
    coreKit::App app;
    
    // Start app
    
    app->start();
    
    // Wait for signal
    
    signals.async_wait([&ioService](const boost::system::error_code &, int) {
        ioService.stop();
    });
    
    ioService.run();
    
    // Exit
    
    return EXIT_SUCCESS;
}
