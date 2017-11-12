//
//  Context.hpp
//  embedded-software
//
//  Created by Pierre Pelé on 10/07/17.
//
//

#pragma once

#include <memory>
#include <thread>
#include <vector>

#include <boost/asio/io_service.hpp>

namespace coreKit {
    
    // Context
    
    class Context {
        
    public:
        
        // Init
        
        Context(size_t threadCount = 1);
        virtual ~Context();
        
        // Access to internal IO service
        
        boost::asio::io_service& getIoService();
        
        // Start / Stop
        
        void start();
        void stop();
        
    private:
        
        // Private methods
        
        // Attributes
        
        size_t                                          _threadCount;
        
        boost::asio::io_service                         _ioService;
        std::unique_ptr<boost::asio::io_service::work>  _ioServiceWork;
        std::vector<std::unique_ptr<std::thread> >      _workers;
        
    };
    
}
