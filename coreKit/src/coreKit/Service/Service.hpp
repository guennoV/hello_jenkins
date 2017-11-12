//
//  Service.hpp
//  embedded-software
//
//  Created by Pierre Pelé on 05/04/17.
//
//

#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace coreKit {
    
    namespace Service {
        
        // Declarations
        
        using Id            = size_t;
        using Name          = std::string;
        using Version       = std::string;
        
        // Opaque delarations
        
        class Handler;
        class Registry;
        
        // ServiceState
        
        enum State {
            
            ServiceUnsubscribed    = 0,
            ServiceIdle            = 1,
            ServiceStarting        = 2,
            ServiceStarted         = 3,
            ServiceStopping        = 4
            
        };
        
        // Status
        
        struct Status {
            
            // Attributes
            
            Name        _name;
            Version     _version;
            State       _state;
            
        };
        
    }
    
    // ServiceBase
    
    class ServiceBase {
        
        friend class Service::Handler;
        friend class Service::Registry;
        
    public:
        
        // Declarations
        
        using Ptr = std::shared_ptr<ServiceBase>;
        
        // Init
        virtual ~ServiceBase();
        
        /* Non-copyable.*/
        ServiceBase(const ServiceBase&) = delete;
        ServiceBase & operator=(const ServiceBase&) = delete;
        
        // Service state
        
        Service::State getState();
        
        // Service info
        
        Service::Name getName();
        Service::Version getVersion();
        
        // Service status
        
        Service::Status getStatus();
        
    protected:
        
        // Init
        
        ServiceBase();
        
        // Service handling
        
        template <typename T> std::shared_ptr<T> getServicePtr();
        
        // Get services status (Including the current service)
        
        std::vector<Service::Status> getServicesStatus();
        
        // Request application closing (Use with care)
        
        void requestAppClosing(const std::string &reason);
        
    private:
        
        // Initialize
        
        void initialize(const Service::Id &serviceId,
                        std::shared_ptr<Service::Registry> registryPtr);
        void deinitialize();
        
        // Start / Stop
        
        void start();
        void stop();
        
        virtual void startService();
        virtual void stopService() noexcept(true);
        
        // Attributes
        
        std::mutex _mutex;
        Service::State _state;
        
        Service::Id _serviceId;
        std::shared_ptr<Service::Registry> _registryPtr;
        
    };
    
}
