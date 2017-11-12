//
//  Log.cpp
//  uavia-software-next
//
//  Created by Pierre Pelé on 18/11/16.
//
//

#include <spdlog/sinks/dist_sink.h>

#include "Log.hpp"

namespace coreKit {
    
    // Internal methods
    
    Log::Log() : _sinks( {
        nullptr, nullptr
    } ) {
        
        // Create sinks
        
        _sinks._intenal = std::make_shared<spdlog::sinks::dist_sink_mt>();
        _sinks._public  = std::make_shared<spdlog::sinks::dist_sink_mt>();
        
    }
    
    Logger::Ptr Log::get_internal(const Logger::Name &loggerName,
                                  Logger::Type type) {
        
        const std::string fullname = Logger::convertName(loggerName);
        Logger::Ptr logger = spdlog::get(fullname);
        
        if (!logger) {
            auto sinkPtr = type == Logger::Type::Public ? _sinks._public : _sinks._intenal;
            logger = spdlog::create(fullname, sinkPtr);
        }
        
        return logger;
    }
    
    void Log::subscribe_internal(const spdlog::sink_ptr &sink,
                                 Logger::Type type) {
        
        auto sinkPtr = type == Logger::Type::Public ? _sinks._public : _sinks._intenal;
        std::static_pointer_cast<spdlog::sinks::dist_sink_mt>(sinkPtr)->add_sink(sink);
    }
    
    void Log::unsubscribe_internal(const spdlog::sink_ptr &sink,
                                   Logger::Type type) {
        auto sinkPtr = type == Logger::Type::Public ? _sinks._public : _sinks._intenal;
        std::static_pointer_cast<spdlog::sinks::dist_sink_mt>(sinkPtr)->remove_sink(sink);
    }
    
    // Static methods
    
    void Log::subscribe(const spdlog::sink_ptr &sink,
                        Logger::Type type) {
        getInstance()->subscribe_internal(sink, type);
    }
    
    void Log::unsubscribe(const spdlog::sink_ptr &sink,
                          Logger::Type type) {
        getInstance()->unsubscribe_internal(sink, type);
    }
    
    void Log::subscribe(const spdlog::sink_ptr &sink) {
        subscribe(sink, Logger::Type::Public);
        subscribe(sink, Logger::Type::Internal);
    }
    
    void Log::unsubscribe(const spdlog::sink_ptr &sink) {
        unsubscribe(sink, Logger::Type::Public);
        unsubscribe(sink, Logger::Type::Internal);
    }
    
    Logger::Ptr Log::get(const Logger::Name &loggerName,
                         Logger::Type type) {
        return getInstance()->get_internal(loggerName, type);
    }
    
    Logger::Ptr Log::get(const Logger::Name &loggerName,
                         Logger::Type type,
                         Logger::Level level) {
        
        Logger::Ptr ptr = get(loggerName, type);
        ptr->set_level(level);
        
        return ptr;
    }
    
}
