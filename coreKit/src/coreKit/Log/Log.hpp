//
//  Log.hpp
//  uavia-software-next
//
//  Created by Pierre Pelé on 18/11/16.
//
//

#pragma once

#include <spdlog/spdlog.h>

#include "Logger.hpp"

#include <coreKit/Utils/Singleton.hpp>

namespace coreKit {
    
    // Log
    
    class Log : private Singleton<Log> {
        
        friend class Singleton<Log>;
        
    public:
        
        // Create / Get logger
        
        static Logger::Ptr get(const Logger::Name &loggerName,
                               Logger::Type type);
        
        static Logger::Ptr get(const Logger::Name &loggerName,
                               Logger::Type type,
                               Logger::Level level);
        
        // Add / Remove sink
        
        static void subscribe(const spdlog::sink_ptr &sink,
                              Logger::Type type);
        static void unsubscribe(const spdlog::sink_ptr &sink,
                                Logger::Type type);
        
        static void subscribe(const spdlog::sink_ptr &sink);
        static void unsubscribe(const spdlog::sink_ptr &sink);
        
    private:
        
        Log();
        
        // Note : Non-copyable by design
        
        void subscribe_internal(const spdlog::sink_ptr &sink,
                                Logger::Type type);
        void unsubscribe_internal(const spdlog::sink_ptr &sink,
                                  Logger::Type type);
        
        Logger::Ptr get_internal(const Logger::Name &loggerName,
                                 Logger::Type type);
        
        // Attributes
        
        struct {
            
            spdlog::sink_ptr _intenal;
            spdlog::sink_ptr _public;
            
        } _sinks;
        
    };
    
}
