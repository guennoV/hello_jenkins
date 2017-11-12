//
//  client.cpp
//  uavia-software-next
//
//

#include <future>
#include <iostream>

#include <boost/asio/signal_set.hpp>
#include <boost/program_options.hpp>

#include <coreKit/Config/Config.hpp>
#include <coreKit/Log/Log.hpp>
#include <coreKit/Utils/StackTrace.hpp>
#include <coreKit/Stream/Client.hpp>

static coreKit::Logger logger( { "coreKit", "SampleStream", "ClientSide"  } );

int main(int argc, const char*argv[]) {
    
    // Add StackTrace ability
    
    coreKit::handleStackTrace();
    
    // Parse command line arguments
    
    std::string remoteUri;
    int timeout_ms;
    
    {
        namespace po        = boost::program_options;
        namespace po_style  = boost::program_options::command_line_style;
        
        po::options_description desc { "Options :" };
        
        // Options definition
        
        desc.add_options()
        ("help,h", "Display this help screen")
        ("uri,u", po::value<std::string>()->default_value("tcp://0.0.0.0:1234"), "Remote URI to be used")
        ("timeout,t", po::value<int>()->default_value(2000), "Connection timeout");
        
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
            
            remoteUri = vm["uri"].as<std::string>();
            timeout_ms = vm["timeout"].as<int>();
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
    
    auto client = coreKit::Stream::makeClient(remoteUri);
    
    // Signal callback
    
    signals.async_wait([&ioService](const boost::system::error_code&,
                                    int) {
        ioService.stop();
    });
    
    // Callbacks
    
    auto onDataReceived = [](const void* data,
                             size_t datalength) {
        
        auto message = std::string((const char*) data, datalength);
        std::cout << "Received message (" << datalength << " Bytes) from Server : '" << message << "'" << std::endl;
    };
    
    auto onDisconnected = [&ioService](const std::exception_ptr cause) {
        
        if (cause) {
            
            std::string errorMsg;
            try {
                std::rethrow_exception(cause);
            } catch (const std::exception &exception) {
                errorMsg = exception.what();
            }
            
            std::cerr << "Client encouters an error : " << errorMsg << std::endl;
            ioService.stop();
            
        }
    };
    
    coreKit::Stream::Client::Callbacks callbacks = {
        onDataReceived, onDisconnected
    };
    
    // Open client
    
    {
        std::promise<void> promise;
        auto future = promise.get_future();
        
        client->open(callbacks, [&promise](const std::exception_ptr error) {
            if (error) {
                promise.set_exception(error);
            } else {
                promise.set_value();
            }
        }, timeout_ms);
        
        future.get();
    }
    
    // Send data
    
    auto task = [&ioService, client]() {
        
        // Write callbacks
        
        coreKit::Stream::Session::WriteCallback writeCallback = [&ioService](const std::exception_ptr error) {
            
            if (error) {
                
                std::string errorMsg;
                try {
                    std::rethrow_exception(error);
                } catch (const std::exception &exception) {
                    errorMsg = exception.what();
                }
                
                std::cerr << "Error while sending message to server : " << errorMsg << std::endl;
                ioService.stop();
            }
        };
        
        std::cout << "Type message to be send to the server :\n"
        << "(Press <Enter> to exit)"
        << std::endl;
        
        std::string msg;
        
        auto getMsg = [](std::string &msg) {
            std::cout << "> ";
            std::getline(std::cin, msg);
        };
        
        for (getMsg(msg); msg != ""; getMsg(msg)) {
            auto buffer = std::make_shared<std::string>(msg);
            client->send(boost::asio::buffer(buffer->c_str(), buffer->size()), [buffer, writeCallback](const std::exception_ptr error) {
                writeCallback(error);
            });
        }
        
        // Quit application
        
        ioService.stop();
    };
    
    // Post task
    
    ioService.post(task);
    
    // Wait for disconnection
    
    ioService.run();
    
    // Exit
    
    return EXIT_SUCCESS;
}
