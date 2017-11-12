//
//  Handle.cpp
//  embedded-software
//
//  Created by Pierre Pelé on 05/04/17.
//
//

#include <coreKit/Utils/iziDeclarations.hpp>

#include "Handle.hpp"

namespace coreKit {
    
    namespace Service {
        
        // Handle
        
        Handle::Handle(const Id &id,
                       const Info &info,
                       const FactoryBase::Ptr factory) :
        
        _id(id),
        _info(info),
        _factory(factory)
        
        { }
        
        Id Handle::getId() const { return _id; }
        Name Handle::getName() const { return _info._name; }
        Version Handle::getVersion() const { return _info._version; }
        Dependencies Handle::getDependencies() const { return _info._dependencies; }
        unsigned int Handle::getOptions() const { return _info._options; }
        
        Info Handle::getInfo() const { return _info; }
        FactoryBase::Ptr Handle::getFactoryPtr() const { return _factory; }
        
        void Handle::buildDependencyContext(DependencyContext &context) const {
            
            const Id id = getId();
            if (context._factories.find(id) == context._factories.end()) {
                
                context._factories.insert(std::make_pair(id, getFactoryPtr()));
                
                const auto name = getName();
                
                if (!coreKit::validateIziUrl(name)) {
                    throw std::runtime_error("Service name [" + name + "] is not izi compliant");
                }
                
                context._names.insert(std::make_pair(id, name));
                context._options.insert(std::make_pair(id, getOptions()));
                
                for (const HandleFactoryBase::Ptr &handleFactory : _info._dependencies) {
                    
                    Handle::Ptr handle = handleFactory->buildHandle();
                    
                    context._edges.push_back(Edge(id, handle->getId()));
                    handle->buildDependencyContext(context);
                }
            }
        }
    }
    
}
