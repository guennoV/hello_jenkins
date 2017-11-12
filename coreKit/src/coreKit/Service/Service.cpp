//
//  Service.cpp
//  embedded-software
//
//  Created by Pierre Pelé on 05/04/17.
//
//

#include "Service.hpp"

#include "Declarations.hpp"
#include "Registry.hpp"

namespace coreKit {
    
    // Base
    
    ServiceBase::ServiceBase() :
    
    _state(Service::ServiceUnsubscribed),
    _serviceId(0),
    _registryPtr(nullptr)
    
    { }
    
    ServiceBase::~ServiceBase() {
        
        if (getState() > Service::ServiceIdle) {
            throw std::runtime_error(std::string("Method [") +
                                     __PRETTY_FUNCTION__ +
                                     "] : Invalid state (" +
                                     std::to_string(_state) + ")");
        }
    }
    
    std::vector<Service::Status> ServiceBase::getServicesStatus() {
        
        Service::Registry::Ptr registryPtr;
        
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_state == Service::ServiceUnsubscribed) {
                throw std::runtime_error(std::string("Method [") +
                                         __PRETTY_FUNCTION__ +
                                         "] : Invalid state (" +
                                         std::to_string(_state) + ")");
            }
            
            registryPtr = _registryPtr;
        }
        
        return registryPtr->getSericesStatus();
    }
    
    void ServiceBase::requestAppClosing(const std::string &reason) {
        
        std::lock_guard<std::mutex> lock(_mutex);
        if (_state == Service::ServiceUnsubscribed) {
            throw std::runtime_error(std::string("Method [") +
                                     __PRETTY_FUNCTION__ +
                                     "] : Invalid state (" +
                                     std::to_string(_state) + ")");
        }
        
        _registryPtr->requestAppClosing(_serviceId, reason);
    }
    
    Service::State ServiceBase::getState() {
        
        std::lock_guard<std::mutex> lock(_mutex);
        return _state;
    }
    
    Service::Name ServiceBase::getName() {
        
        std::lock_guard<std::mutex> lock(_mutex);
        if (_state > Service::ServiceUnsubscribed) {
            return _registryPtr->getServiceName(_serviceId);
        } else {
            throw std::runtime_error(std::string("Method [") +
                                     __PRETTY_FUNCTION__ +
                                     "] : Invalid state (" +
                                     std::to_string(_state) + ")");
        }
    }
    
    Service::Version ServiceBase::getVersion() {
        
        std::lock_guard<std::mutex> lock(_mutex);
        if (_state > Service::ServiceUnsubscribed) {
            return _registryPtr->getServiceVersion(_serviceId);
        } else {
            throw std::runtime_error(std::string("Method [") +
                                     __PRETTY_FUNCTION__ +
                                     "] : Invalid state (" +
                                     std::to_string(_state) + ")");
        }
    }
    
    Service::Status ServiceBase::getStatus() {
        
        std::lock_guard<std::mutex> lock(_mutex);
        if (_state > Service::ServiceUnsubscribed) {
            return {
                _registryPtr->getServiceName(_serviceId),
                _registryPtr->getServiceVersion(_serviceId),
                _state
            };
            
        } else {
            throw std::runtime_error(std::string("Method [") +
                                     __PRETTY_FUNCTION__ +
                                     "] : Invalid state (" +
                                     std::to_string(_state) + ")");
        }
    }
    
    void ServiceBase::initialize(const Service::Id &serviceId, std::shared_ptr<Service::Registry> registryPtr) {
        
        std::lock_guard<std::mutex> lock(_mutex);
        if (_state != Service::ServiceUnsubscribed) {
            throw std::runtime_error(std::string("Method [") +
                                     __PRETTY_FUNCTION__ +
                                     "] : Invalid state (" +
                                     std::to_string(_state) + ")");
        }
        
        _serviceId = serviceId;
        _registryPtr = registryPtr;
        
        _state = Service::ServiceIdle;
    }
    
    void ServiceBase::deinitialize() {
        
        std::lock_guard<std::mutex> lock(_mutex);
        if (_state != Service::ServiceIdle) {
            throw std::runtime_error(std::string("Method [") +
                                     __PRETTY_FUNCTION__ +
                                     "] : Invalid state (" +
                                     std::to_string(_state) + ")");
        }
        
        _serviceId = 0;
        _registryPtr = nullptr;
        
        _state = Service::ServiceUnsubscribed;
    }
    
    void ServiceBase::start() {
        
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_state != Service::ServiceIdle) {
                throw std::runtime_error(std::string("Method [") +
                                         __PRETTY_FUNCTION__ +
                                         "] : Invalid state (" +
                                         std::to_string(_state) + ")");
            }
            
            _state = Service::ServiceStarting;
        }
        
        std::exception_ptr exceptionPtr = nullptr;
        
        try {
            startService();
        } catch (...) {
            exceptionPtr = std::current_exception();
        }
        
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (exceptionPtr) {
                _state = Service::ServiceIdle;
                std::rethrow_exception(exceptionPtr);
            } else {
                _state = Service::ServiceStarted;
            }
        }
    }
    
    void ServiceBase::stop() {
        
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_state == Service::ServiceIdle) {
                
                // There's nothing to do
                return;
                
            } else if (_state != Service::ServiceStarted) {
                throw std::runtime_error(std::string("Method [") +
                                         __PRETTY_FUNCTION__ +
                                         "] : Invalid state (" +
                                         std::to_string(_state) + ")");
            }
            
            _state = Service::ServiceStopping;
        }
        
        try {
            stopService();
        } catch (...) {
            Service::_serviceLogger->critical("Service [{}] throw an exception while stopping. This breaks coreKit © design rules", getName());
            std::rethrow_exception(std::current_exception());
        }
        
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _state = Service::ServiceIdle;
        }
    }
    
    void ServiceBase::startService() { /* Nothing to do */ }
    void ServiceBase::stopService() noexcept(true) { /* Nothing to do */ }
    
}
