//
//  main.cpp
//  uavia-software-next
//
//

#include <coreKit/Log/Logger.hpp>
#include <coreKit/Log/Log.hpp>

static coreKit::Logger logger( { "coreKit", "SampleLogger"  } );

int main() {
    
    // Add log sink
    
    {
        bool useColor = true;
        
        spdlog::sink_ptr sink = std::make_shared<spdlog::sinks::stdout_sink_mt>();
        
        if (useColor) {
            sink = std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>();
        }
        
        coreKit::Log::subscribe(sink);
    }
    
    // Log
    
    {
        logger->set_level(spdlog::level::trace);
        
        logger->info     ("Welcome to coreKit !");
        logger->warn     ("Welcome to coreKit !");
        logger->debug    ("Welcome to coreKit !");
        logger->trace    ("Welcome to coreKit !");
        logger->error    ("Welcome to coreKit !");
    }
    
    // Exit
    
    return EXIT_SUCCESS;
}
