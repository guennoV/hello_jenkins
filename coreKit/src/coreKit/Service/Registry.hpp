//
//  Registry.hpp
//  uavia-software-next
//
//  Created by Pierre Pelé on 05/01/17.
//
//

#pragma once

#include <map>
#include <memory>

#include <boost/thread/shared_mutex.hpp>

#include "Handle.hpp"

namespace coreKit {
    
    namespace Service {
        
        // Opaque declarations
        
        class Handler;
        
        // Container
        
        struct Container {
            
            // Methods
            
            Container(ServiceBase::Ptr ptr, const Handle::Ptr handle);
            Container(const FactoryBase::Ptr factory);
            
            template <typename T> Container(std::shared_ptr<T> basePtr);
            
            Id getId() const;
            Name getName() const;
            Version getVersion() const;
            
            std::string formatServiceInfo() const;
            
            // Attributes
            
            ServiceBase::Ptr _ptr;
            const Handle::Ptr _handle;
            
        };
        
        // Registry
        
        class Registry {
            
            friend class Handler;
            
        public:
            
            // Declarations
            
            using Ptr = std::shared_ptr<Registry>;
            
            // Init
            
            Registry(const std::function<void(const std::string serviceName,
                                              const std::string &reason)> &onAppCloseRequest);
            
            /* Non-copyable.*/
            Registry(const Registry&) = delete;
            Registry & operator=(const Registry&) = delete;
            
            // Access to services
            
            template <typename T> std::shared_ptr<T> getServicePtr();
            
            // Get service infos
            
            Name getServiceName(const Id &id);
            Version getServiceVersion(const Id &id);
            
            // Start service
            
            void startService(const Id &id);
            
            // Stop service
            
            void stopService(const Id &id);
            
            // For log purpose only
            
            std::string formatServiceInfo(const Id &id);
            
            // Get services status
            
            std::vector<Status> getSericesStatus();
            
            // Static methods
            
            template <typename T> static Id getId();
            template <typename T> static Info getInfo();
            
            template <typename T> static std::string formatServiceInfo();
            
            // Request application closing (Use with care)
            
            void requestAppClosing(const Id &id, const std::string &reason);
            
            // Helpers
            
            static std::string formatServiceInfo(const Name &name, const Id &id);
            
        private:
            
            // Private methods
            
            void clear();
            
            void subscribe(const Container &container);
            ServiceBase::Ptr getServicePtr(const Id &id);
            
            DependencyGraph buildDependencyGraph();
            
            // Attributes
            
            const std::function<void(const std::string serviceName,
                                     const std::string &reason)> _onAppCloseRequest;
            
            boost::shared_mutex _mutex;
            std::map<Id, Container> _containers;
            
        };
        
    }
    
}

#include "Registry.ipp"
