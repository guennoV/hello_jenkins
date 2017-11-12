//
//  CoreAPI.hpp
//  uavia-software-next
//
//  Created by Pierre Pelé on 02/01/17.
//
//

#pragma once

#include <memory>
#include <mutex>
#include <string>

#include <coreKit/Service/Handler.hpp>

namespace coreKit {
    
    // Opaque declarations
    
    class App;
    
    namespace Application {
        
        // CoreAPI
        
        class CoreAPI {
            
        public:
            
            using Ptr = std::shared_ptr<CoreAPI>;
            
            // Init
            
            CoreAPI(const Service::HandlerCallbacks &callbacks);
            
            /* Non-copyable.*/
            
            CoreAPI(const CoreAPI&) = delete;
            CoreAPI & operator=(const CoreAPI&) = delete;
            
            // Subscribe / Get a service
            
            template <class T, class... Args> std::shared_ptr<T> subscribe(Args&&... args);
            template <typename T> std::shared_ptr<T> getServicePtr();
            
            // Start / Stop CoreAPI
            
            void start();
            void stop();
            
        private:
            
            // Attributes
            
            // Service handler
            
            Service::Handler::Ptr   _handlerPtr;
            
        };
        
        template <class T, class... Args> std::shared_ptr<T> CoreAPI::subscribe(Args&&... args) {
            return _handlerPtr->subscribe<T>(std::forward<Args>(args)...);
        }
        
        template <typename T> std::shared_ptr<T> CoreAPI::getServicePtr() {
            return _handlerPtr->getServicePtr<T>();
        }
    }
}
