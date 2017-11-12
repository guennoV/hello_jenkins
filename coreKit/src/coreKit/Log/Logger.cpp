//
//  Logger.cpp
//  uavia-software-next
//
//  Created by Pierre Pelé on 17/11/16.
//
//

#include <coreKit/Utils/iziDeclarations.hpp>

#include "Logger.hpp"

#include "Log.hpp"

namespace coreKit {
    
    // Logger implementation
    
    Logger::Logger(const Name &name,
                   Type type,
                   Level level) :
    
    _created    (0),
    _ptr        (nullptr)
    
    {
        configure(name, type, level);
    }
    
    void Logger::configure(const Name &name,
                           Type type,
                           Level level) {
        
        if (_name.size() > 0) {
            throw std::runtime_error("Logger is already configured");
        }
        
        if (name.size() > 0) {
            
            if (!coreKit::validateIziUrl(convertName(name))) {
                throw std::runtime_error("Logger name [" + convertName(name) + "] is not izi compliant");
            }
            
            _name       = name;
            _type       = type;
            _level      = level;
        }
    }
    
    std::string Logger::convertName(const Name &name) {
        
        std::string result;
        for (Name::const_iterator it = name.begin(); it != name.end(); it++) {
            result += (std::distance(name.begin(), it) != 0 ? "." : "") + *it;
        }
        
        return result;
    }
    
    Logger::Ptr Logger::operator->() {
        
        if (!_created.load()) {
            std::lock_guard<std::mutex> lock(_mutex);
            if (!_created.load()) {
                if (_name.size() > 0) {
                    _ptr = Log::get(_name, _type, _level);
                    _created = 1;
                } else {
                    throw std::runtime_error("Logger is not configured");
                }
            }
        }
        
        return _ptr;
    }
    
}
