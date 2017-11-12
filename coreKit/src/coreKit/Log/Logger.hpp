//
//  Logger.hpp
//  uavia-software-next
//
//  Created by Pierre Pelé on 17/11/16.
//
//

#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <spdlog/logger.h>

namespace coreKit {
    
    // Opaque declarations
    
    class Log;
    
    // Logger
    
    class Logger {
        
        friend class Log;
        
    public:
        
        // Declarations
        
        using Name  = std::vector<std::string>;
        using Ptr   = std::shared_ptr<spdlog::logger>;
        using Level = spdlog::level::level_enum;
        
        enum class Type {
            
            Public      = 0,
            Internal    = 1
            
        };
        
        // Init
        
        Logger(const Name &name,
               Type type = Type::Public,
               Level level = spdlog::level::trace);
        void configure(const Name &name,
                       Type type = Type::Public,
                       Level level = spdlog::level::trace);
        
        Ptr operator->();
        
    private:
        
        static std::string convertName(const Name &name);
        
        // Attributes
        
        std::atomic_uchar _created;
        std::mutex _mutex;
        
        Ptr _ptr;
        
        Name    _name;
        Type    _type;
        Level   _level;
        
    };
    
}
