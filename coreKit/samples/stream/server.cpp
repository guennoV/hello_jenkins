//
//  server.cpp
//  uavia-software-next
//
//

#include <iostream>

#include <boost/asio/signal_set.hpp>
#include <boost/program_options.hpp>

#include <coreKit/Config/Config.hpp>
#include <coreKit/Log/Log.hpp>
#include <coreKit/Utils/StackTrace.hpp>
#include <coreKit/Stream/Server.hpp>

static coreKit::Logger logger( { "coreKit", "SampleStream", "ServerSide"  } );

int main(int argc, const char*argv[]) {
    
    // Add StackTrace ability
    
    coreKit::handleStackTrace();
    
    // Parse command line arguments
    
    std::string localUri;
    
    {
        namespace po        = boost::program_options;
        namespace po_style  = boost::program_options::command_line_style;
        
        po::options_description desc { "Options :" };
        
        // Options definition
        
        desc.add_options()
        ("help,h", "Display this help screen")
        ("uri,u", po::value<std::string>()->default_value("tcp://0.0.0.0:1234"), "Local URI to be used");
        
        // Boost program options initialization
        
        po::variables_map vm;
        
        {
            po::store(po::command_line_parser(argc, argv).options(desc).style(po_style::unix_style | po_style::case_insensitive).run(), vm);
            po::store(parse_command_line(argc, argv, desc), vm);
            po::notify(vm);
            
            if (vm.count("help")) {
                std::cout << desc << std::endl;
                
                return EXIT_SUCCESS;
            }
            
            localUri = vm["uri"].as<std::string>();
        }
    }
    
    // Add log sink
    
    {
        spdlog::sink_ptr sink = std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>();
        coreKit::Log::subscribe(sink);
    }
    
    // Parse environment
    
    coreKit::Config::parseEnv();
    
    // Signal handling
    
    boost::asio::io_service ioService;
    boost::asio::signal_set signals(ioService, SIGINT, SIGTERM);
    
    // Client init
    
    auto server = coreKit::Stream::makeServer(ioService, localUri);
    
    // Signal callback
    
    signals.async_wait([server](const boost::system::error_code&, int) {
        server->close(nullptr);
    });
    
    // Callbacks
    
    auto onDataReceived = [&ioService, server](const coreKit::Stream::Server::SessionId &sessionId,
                                               const void* data,
                                               size_t datalength) {
        
        auto message = std::make_shared<std::string>((const char*) data, datalength);
        std::cout << "Received message (" << datalength << " Bytes) from session with ID N°" << sessionId << " : " << message->c_str() << std::endl;
        
        server->send(sessionId, boost::asio::buffer(message->c_str(), message->size()), [&ioService, sessionId, message](const std::exception_ptr error) {
            
            if (error) {
                
                std::string errorMsg;
                try {
                    std::rethrow_exception(error);
                } catch (const std::exception &exception) {
                    errorMsg = exception.what();
                }
                
                std::cerr << "Error while sending message to session with ID N°" << sessionId << " : " << errorMsg << std::endl;
                ioService.stop();
            }
        });
    };
    
    auto onConnected = [&ioService](const std::exception_ptr error) {
        
        if (error) {
            
            std::string errorMsg;
            try {
                std::rethrow_exception(error);
            } catch (const std::exception &exception) {
                errorMsg = exception.what();
            }
            
            std::cerr << "Connection error : " << errorMsg << std::endl;
            
            ioService.stop();
            
        } else {
            std::cout << "Server is now connected" << std::endl;
        }
    };
    
    auto onClosed = [&ioService](const std::exception_ptr cause) {
        
        if (cause) {
            
            std::string errorMsg;
            try {
                std::rethrow_exception(cause);
            } catch (const std::exception &exception) {
                errorMsg = exception.what();
            }
            
            std::cerr << "Server encouters an error : " << errorMsg << std::endl;
        } else {
            std::cout << "Server is now disconnected" << std::endl;
        }
        
        ioService.stop();
    };
    
    auto onNewSession = [](const coreKit::Stream::Server::SessionId &sessionId) {
        std::cout << "New session with ID N°" << sessionId << std::endl;
    };
    
    auto onSessionDisconnected = [](const coreKit::Stream::Server::SessionId &sessionId,
                                    const std::exception_ptr) {
        std::cout << "Session with ID N°" << sessionId << " destructed" << std::endl;
    };
    
    coreKit::Stream::Server::Callbacks callbacks = {
        onDataReceived, onClosed, onNewSession, onSessionDisconnected
    };
    
    // Open server
    
    server->accept(callbacks, onConnected);
    
    // Wait for disconnection
    
    ioService.run();
    
    // Exit
    
    return EXIT_SUCCESS;
}
