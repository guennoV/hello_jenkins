//
//  forwardSink.hpp
//  embedded-software
//
//  Created by Mathieu Corti on 10/04/17.
//
//

#pragma once

#include <functional>
#include <memory>
#include <mutex>

#include <spdlog/logger.h>
#include <spdlog/fmt/bundled/format.h>

namespace coreKit {
    
    // ForwardSink
    
    template <class Mutex>
    class ForwardSink :
    public spdlog::sinks::base_sink<Mutex> {
        
    public:
        
        // Declarations
        
        using Msg = spdlog::details::log_msg;
        using Ptr = std::shared_ptr<ForwardSink<Mutex> >;
        
        // Init
        
        ForwardSink(const std::function<void(const Msg&)> &userCallback) : _userCallback(userCallback) {
            if (!userCallback) {
                throw std::runtime_error("User callback must be defined");
            }
        }
        
    private:
        
        void _sink_it(const spdlog::details::log_msg &msg) override {
            _userCallback(msg);
        }
        
        void _flush() override { /* Nothing to do here */  }
        
        // Attributes
        
        const std::function<void(const Msg&)> _userCallback;
    };
    
    // Helpers
    
    using ForwardSink_mt = ForwardSink<std::mutex>;
}
