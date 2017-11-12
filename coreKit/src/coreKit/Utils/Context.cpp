//
//  Context.cpp
//  embedded-software
//
//  Created by Pierre Pelé on 10/07/17.
//
//

#include "Context.hpp"

namespace coreKit {
    
    // ioService
    
    Context::Context(size_t threadCount) : _threadCount(threadCount), _ioServiceWork(nullptr) { }
    Context::~Context() { stop(); }
    
    boost::asio::io_service& Context::getIoService() {
        return _ioService;
    }
    
    void Context::start() {
        
        if (!_ioServiceWork) {
            
            _ioServiceWork = std::unique_ptr<boost::asio::io_service::work>(new boost::asio::io_service::work(_ioService));
            
            for (size_t i = 0; i < _threadCount; i++) {
                _workers.push_back(std::unique_ptr<std::thread>(new std::thread(std::bind(static_cast<std::size_t(boost::asio::io_service::*)()>(&boost::asio::io_service::run), &_ioService))));
            }
        }
    }
    
    void Context::stop() {
        
        if (_ioServiceWork) {
            
            _ioServiceWork = nullptr;
            
            for (auto &worker : _workers) {
                worker->join();
            }
            
            _workers.clear();
        }
    }
    
}
