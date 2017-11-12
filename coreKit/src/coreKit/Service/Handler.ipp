//
//  Handler.ipp
//  uavia-software-next
//
//  Created by Pierre Pelé on 05/01/17.
//
//

#pragma once

#include "Handler.hpp"

#include "Declarations.hpp"

namespace coreKit {
    
    namespace Service {
        
        // Public methods
        
        template <class T, class... Args> std::shared_ptr<T> Handler::subscribe(Args&&... args) {
            
            std::lock_guard<std::mutex> lock(_mutex);
            
            if (_state != HandlerIdle) {
                throw std::runtime_error(std::string("Method [") + __PRETTY_FUNCTION__ + "] : Invalid state (" + std::to_string(_state) + ")");
            }
            
            return subscribe_internal<T>(std::forward<Args>(args)...);
        }
        
        template <typename T> std::shared_ptr<T> Handler::getServicePtr() {
            
            std::lock_guard<std::mutex> lock(_mutex);
            
            if (_state != HandlerStarted) {
                throw std::runtime_error(std::string("Method [") + __PRETTY_FUNCTION__ + "] : Invalid state (" + std::to_string(_state) + ")");
            }
            
            return getServicePtr_internal<T>();
        }
        
        // Private methods
        
        template <class T, class... Args> std::shared_ptr<T> Handler::subscribe_internal(Args&&... args) {
            
            _serviceLogger->debug("Creating service {}", Registry::formatServiceInfo<T>());
            
            std::shared_ptr<T> ptr = std::make_shared<T>(std::forward<Args>(args)...);
            subscribe_internal(Container(ptr));
            
            return ptr;
        }
        
        template <typename T> std::shared_ptr<T> Handler::getServicePtr_internal() {
            return _registryPtr->getServicePtr<T>();
        }
        
    }
}
