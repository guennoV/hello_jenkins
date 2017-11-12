//
//  Handle.hpp
//  embedded-software
//
//  Created by Pierre Pelé on 05/04/17.
//
//

#pragma once

#include <map>
#include <vector>

#include "Dependency.hpp"

namespace coreKit {
    
    namespace Service {
        
        // Handle
        
        class Handle {
            
        public:
            
            // Declarations
            
            using Ptr = std::shared_ptr<Handle>;
            
            // Init
            
            Handle(const Id &id,
                   const Info &info,
                   const FactoryBase::Ptr factory);
            
            // Get service informations
            
            Id getId() const;
            Name getName() const;
            Version getVersion() const;
            Dependencies getDependencies() const;
            unsigned int getOptions() const;
            
            // Build dependency graph
            
            void buildDependencyContext(DependencyContext &context) const;
            
        private:
            
            // Private methods
            
            Info getInfo() const;
            FactoryBase::Ptr getFactoryPtr() const;
            
            // Attributes
            
            const Id _id;
            const Info _info;
            const FactoryBase::Ptr _factory;
            
        };
        
        // Helper methods
        
        template <typename T> Handle::Ptr makeHandle();
        
    }
}
