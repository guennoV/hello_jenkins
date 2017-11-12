//
//  Dependency.hpp
//  embedded-software
//
//  Created by Pierre Pelé on 05/04/17.
//
//

#pragma once

#include <map>
#include <vector>
#include <utility>

#include "Service.hpp"

namespace coreKit {
    
    namespace Service {
        
        // Declarations
        
        using Edge          = std::pair<Id, Id>;
        using StartOrder    = std::vector<std::vector<Id> >;
        
        // Opaque delarations
        
        class Handle;
        
        // HandleFactoryBase
        
        class HandleFactoryBase {
        public:
            using Ptr = std::shared_ptr<HandleFactoryBase>;
            virtual std::shared_ptr<Handle> buildHandle() const = 0;
        };
        
        // Helper methods
        
        template <typename T> HandleFactoryBase::Ptr makeDep();
        
        // HandleFactory
        
        template <typename T> class HandleFactory : public HandleFactoryBase {
        public:
            std::shared_ptr<Handle> buildHandle() const override;
        };
        
        // FactoryBase
        
        class FactoryBase {
            
        public:
            
            // Declarations
            
            using Ptr = std::shared_ptr<FactoryBase>;
            
            // Methods
            
            virtual ServiceBase::Ptr buildService() const = 0;
            virtual std::shared_ptr<Handle> buildHandle() const = 0;
            
        };
        
        // Factory
        
        template <typename T>
        class Factory : public FactoryBase {
            
        public:
            
            Factory();
            ServiceBase::Ptr buildService() const override {
                return std::make_shared<T>();
            }
            
            std::shared_ptr<Handle> buildHandle() const override;
            
        private:
            
            const HandleFactoryBase::Ptr _handleFactory;
        };
        
        // Helper methods
        
        template <typename T> FactoryBase::Ptr makeFactory() {
            return std::make_shared<Factory<T> >();
        }
        
        // Dependencies
        
        using Dependencies = std::vector<HandleFactoryBase::Ptr>;
        
        // Options
        
        enum Options {
            
            // Start / Stop methods must be executed on the calling thread
            // (Do not create thread(s) to start / stop service)
            
            DefferedStartStop = 1
            
        };
        
        // Info
        
        struct Info {
            
            Name            _name;
            Version         _version;
            
            Dependencies    _dependencies;
            
            unsigned int    _options;
            
        };
        
        // DependencyContext
        
        struct DependencyContext {
            
            // Methods
            
            std::string toString() const;
            
            // For log purpose only
            
            std::string formatServiceInfo(const Id &id) const;
            
            // Attrbutes
            
            std::vector<Edge>               _edges;
            std::map<Id, FactoryBase::Ptr>  _factories;
            std::map<Id, Name>              _names;
            std::map<Id, unsigned int>      _options;
            
        };
        
        // DependencyGraph
        
        struct DependencyGraph {
            
            // Methods
            
            std::string toString() const;
            
            // Attributes
            
            DependencyContext   _context;
            StartOrder          _startOrder;
            
        };
        
    }
}
