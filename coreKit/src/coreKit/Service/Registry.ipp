//
//  Registry.ipp
//  uavia-software-next
//
//  Created by Pierre Pelé on 05/01/17.
//
//

#pragma once

#include "Registry.hpp"

#include <typeinfo>

namespace coreKit {
    
    namespace Service {
        
        // Registry
        
        template <typename T> std::shared_ptr<T> Registry::getServicePtr() {
            
            const Id id = getId<T>();
            ServiceBase::Ptr ptr = getServicePtr(id);
            
            if (ptr == nullptr) {
                throw std::runtime_error(std::string("Method [") +
                                         __PRETTY_FUNCTION__ +
                                         "] : Unavailable service with ID '" +
                                         std::to_string(id) + "'");
            }
            
            return std::dynamic_pointer_cast<T>(ptr);
        }
        
        template <typename T> Id Registry::getId() {
            return typeid(T).hash_code();
        }
        
        template <typename T> Info Registry::getInfo() {
            return T::getInfo();
        }
        
        template <typename T> std::string Registry::formatServiceInfo() {
            return Registry::formatServiceInfo(getInfo<T>()._name, getId<T>());
        }
        
        // Helper methods
        
        template <typename T> HandleFactoryBase::Ptr makeDep() {
            return std::make_shared<HandleFactory<T> >();
        }
        
        template <typename T> Handle::Ptr makeHandle() {
            return std::make_shared<Handle>(Registry::getId<T>(), Registry::getInfo<T>(), makeFactory<T>());
        };
        
        // HandleFactory
        
        template <typename T> std::shared_ptr<Handle> HandleFactory<T>::buildHandle() const {
            return makeHandle<T>();
        }
        
        // Factory
        
        template <typename T> Factory<T>::Factory() : _handleFactory(makeDep<T>()) { }
        
        template <typename T> std::shared_ptr<Handle> Factory<T>::buildHandle() const {
            return _handleFactory->buildHandle();
        }
        
        // Container
        
        template <typename T> Container::Container(std::shared_ptr<T> basePtr) : Container(basePtr, makeHandle<T>()) { }
        
    }
    
    // ServiceBase
    
    template <typename T> std::shared_ptr<T> ServiceBase::getServicePtr() {
        
        if (_registryPtr) {
            return _registryPtr->getServicePtr<T>();
        }
        
        throw std::runtime_error("Current service is unsubscribed");
    }
    
}
