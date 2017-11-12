//
//  Handler.hpp
//  uavia-software-next
//
//  Created by Pierre Pelé on 05/01/17.
//
//

#pragma once

#include "Registry.hpp"

#include <boost/optional.hpp>

namespace coreKit {
    
    namespace Service {
        
        // HandlerState
        
        enum HandlerState {
            
            HandlerIdle         = 0,
            HandlerStarting     = 1,
            HandlerStarted      = 2,
            HandlerStopping     = 3
            
        };
        
        // HandlerStatus
        
        struct HandlerStatus {
            
            // Methods
            
            std::string toString() const;
            
            // Attributes
            
            HandlerState            _state;
            std::vector<Status>     _services;
            
        };
        
        // HandlerCallbacks
        
        struct HandlerCallbacks {
            
            // Methods
            
            HandlerCallbacks();
            
            // Attributes;
            
            // Function to be called when a specific service request App Closing
            // This happens when a service encounters a critical error
            // Note : This method can be called several times
            
            std::function<void(const std::string serviceName,
                               const std::string &reason)> _onAppCloseRequest;
            
        };
        
        // Handler
        
        class Handler {
            
        public:
            
            // Declarations
            
            using Ptr = std::shared_ptr<Handler>;
            
            // Init
            
            Handler(const HandlerCallbacks &callbacks);
            virtual ~Handler();
            
            /* Non-copyable.*/
            Handler(const Handler&) = delete;
            Handler & operator=(const Handler&) = delete;
            
            // Subscribe / Get a service
            
            template <class T, class... Args> std::shared_ptr<T> subscribe(Args&&... args);
            template <typename T> std::shared_ptr<T> getServicePtr();
            
            // Start / Stop all services
            
            void start();
            void stop();
            
            // Handler state
            
            HandlerState getState();
            
            // Handler status
            
            HandlerStatus getStatus();
            
        private:
            
            // Private methods
            
            void start_internal();
            
            void stop_internal();
            void stop_internal(size_t timeSlot);
            
            template <class T, class... Args> std::shared_ptr<T> subscribe_internal(Args&&... args);
            template <typename T> std::shared_ptr<T> getServicePtr_internal();
            
            ServiceBase::Ptr subscribe_internal(Container container);
            ServiceBase::Ptr getServicePtr_internal(const Id &id);
            
            void startService_internal(const Id &id, const FactoryBase::Ptr factoryPtr);
            void stopService_internal(const Id &id);
            
            // Attributes
            
            const HandlerCallbacks              _callbacks;
            
            std::mutex                          _mutex;
            HandlerState                        _state;
            
            std::shared_ptr<Registry>           _registryPtr;
            boost::optional<DependencyGraph>    _graph;
            
        };
        
    }
}

#include "Handler.ipp"
