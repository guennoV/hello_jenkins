//
//  Handler.cpp
//  uavia-software-next
//
//  Created by Pierre Pelé on 05/01/17.
//
//

#include "Handler.hpp"

#include <future>

#include <coreKit/Config/Config.hpp>

#include "Declarations.hpp"

static const coreKit::ConfigList configList({ "coreKit", "Service" }, {
    
    //                      | Name                  | Description                                           | Default value
    
    coreKit::makeParam<bool>("AsyncLaunchPolicy",   "Should we autorize async start / stop for services",   true)
    
});

namespace coreKit {
    
    namespace Service {
        
        // Helpers
        
        static std::launch getLaunchPolicy() {
            
            std::launch policy;
            if (Config::get<bool>("coreKit.Service.AsyncLaunchPolicy")) {
                policy = std::launch::deferred | std::launch::async ;
            } else {
                policy = std::launch::deferred;
            }
            
            return policy;
        }
        
        // HandlerStatus
        
        std::string HandlerStatus::toString() const {
            
            std::string result;
            
            if (_services.size() == 0) {
                result += "\n\t--- No service ---";
            } else {
                for (const Status &status : _services) {
                    result += "\n\t" + status._name + " | " + status._version;
                }
            }
            
            return result;
        }
        
        // HandlerCallbacks
        
        HandlerCallbacks::HandlerCallbacks() : _onAppCloseRequest(nullptr) { }
        
        // Handler
        
        Handler::Handler(const HandlerCallbacks &callbacks) :
        
        _callbacks      (callbacks),
        _state          (HandlerIdle),
        _registryPtr    (std::make_shared<Registry>(callbacks._onAppCloseRequest))
        
        { }
        
        Handler::~Handler() {
            
            std::lock_guard<std::mutex> lock(_mutex);
            if (_state != HandlerIdle) {
                throw std::runtime_error(std::string("Method [") + __PRETTY_FUNCTION__ + "] : Invalid state (" + std::to_string(_state) + ")");
            }
            
            _registryPtr->clear();
        }
        
        void Handler::start() {
            
            {
                std::lock_guard<std::mutex> lock(_mutex);
                
                if (_state == HandlerStarted) {
                    return;
                } else if (_state != HandlerIdle) {
                    throw std::runtime_error(std::string("Method [") + __PRETTY_FUNCTION__ + "] : Invalid state (" + std::to_string(_state) + ")");
                }
                
                _state = HandlerStarting;
            }
            
            std::exception_ptr exceptionPtr = nullptr;
            
            try {
                start_internal();
            } catch (...) {
                exceptionPtr = std::current_exception();
            }
            
            {
                std::lock_guard<std::mutex> lock(_mutex);
                if (exceptionPtr) {
                    _state = HandlerIdle;
                    std::rethrow_exception(exceptionPtr);
                } else {
                    _state = HandlerStarted;
                }
            }
        }
        
        void Handler::start_internal() {
            
            _serviceLogger->info("Starting Service handler ..");
            
            // Build the dependency graph
            auto graph = _registryPtr->buildDependencyGraph();
            _serviceLogger->debug("Dependency graph : \n{}", graph.toString());
            
            if (graph._startOrder.size() == 0) {
                _serviceLogger->warn("No service to start");
                
            } else {
                
                // Save the grah
                _graph.reset(graph);
                
                const auto globalPolicy = getLaunchPolicy();
                const auto &startOrder  = _graph->_startOrder;
                const auto &context     = _graph->_context;
                
                size_t slot = 0;
                for (StartOrder::const_iterator it_vec = startOrder.begin(); it_vec != startOrder.end(); it_vec++) {
                    
                    _serviceLogger->debug("Starting services attached to time slot N°{}", slot);
                    
                    std::map<Id, std::exception_ptr> errors;
                    
                    if (it_vec->size() == 1) {
                        
                        const Id id = it_vec->front();
                        
                        try {
                            startService_internal(id, context._factories.find(id)->second);
                        } catch (...) {
                            errors.insert(std::make_pair(id, std::current_exception()));
                        }
                        
                    } else {
                        
                        std::map<Id, std::future<void> > futures;
                        for (std::vector<Id>::const_iterator it = it_vec->begin(); it != it_vec->end(); it++) {
                            
                            const Id id = *it;
                            
                            std::launch policy = globalPolicy;
                            if (context._options.find(id)->second & Service::DefferedStartStop) {
                                _serviceLogger->info("Service {} requested deffered start / stop",
                                                     context.formatServiceInfo(id));
                                policy = std::launch::deferred;
                            }
                            
                            futures.insert(std::make_pair(id, std::async(policy, &Handler::startService_internal, this, id, context._factories.find(id)->second)));
                        }
                        
                        for (std::map<Id, std::future<void> >::iterator it = futures.begin(); it != futures.end(); it++) {
                            
                            const Id id = it->first;
                            
                            try {
                                it->second.get();
                            } catch (...) {
                                errors.insert(std::make_pair(id, std::current_exception()));
                            }
                        }
                    }
                    
                    if (errors.size() > 0) {
                        
                        std::string lastKnownServiceInfo;
                        std::string lastKnownReason;
                        
                        for (std::map<Id, std::exception_ptr>::const_iterator it = errors.begin(); it != errors.end(); it++) {
                            
                            lastKnownServiceInfo = context.formatServiceInfo(it->first);
                            
                            try {
                                std::rethrow_exception(it->second);
                            } catch(const std::exception &exception) {
                                lastKnownServiceInfo = context.formatServiceInfo(it->first);
                                lastKnownReason = exception.what();
                            } catch (...) {
                                lastKnownReason = "Unknown error | Please check use of exception(s)";
                            }
                            
                            _serviceLogger->error("An error occurs while starting service {} : [{}]",
                                                  lastKnownServiceInfo,
                                                  lastKnownReason);
                        }
                        
                        stop_internal(slot);
                        
                        if (errors.size() == 1) {
                            
                            // Throw a detailed error
                            throw std::runtime_error("An error occurs while starting service " +
                                                     lastKnownServiceInfo +
                                                     " : [" + lastKnownReason + "]");
                        } else {
                            
                            // Throw a generic error
                            throw std::runtime_error("An error occurs for " + std::to_string(errors.size()) + " service(s) at startup");
                        }
                        
                    }
                    
                    slot++;
                }
            }
            
            _serviceLogger->info("Service handler is now started");
        }
        
        void Handler::stop() {
            
            {
                std::lock_guard<std::mutex> lock(_mutex);
                if (_state == HandlerIdle) {
                    return;
                } else if (_state != HandlerStarted) {
                    throw std::runtime_error(std::string("Method [") + __PRETTY_FUNCTION__ + "] : Invalid state (" + std::to_string(_state) + ")");
                }
                
                _state = HandlerStopping;
            }
            
            stop_internal();
            
            {
                std::lock_guard<std::mutex> lock(_mutex);
                _state = HandlerIdle;
            }
        }
        
        void Handler::stop_internal() {
            
            _serviceLogger->info("Stopping service handler ..");
            
            if (_graph) {
                
                size_t timeSlotCount = _graph->_startOrder.size();
                if (timeSlotCount > 0) {
                    stop_internal(timeSlotCount - 1);
                }
                
            } else {
                _serviceLogger->warn("No service to stop");
            }
            
            _serviceLogger->info("Service handler is now stopped");
        }
        
        void Handler::stop_internal(size_t timeSlot) {
            
            const auto globalPolicy = getLaunchPolicy();
            const auto &startOrder  = _graph->_startOrder;
            const auto &context     = _graph->_context;
            
            size_t slot = timeSlot;
            
            while(true) {
                
                StartOrder::const_iterator it_vec = startOrder.begin();
                std::advance(it_vec, slot);
                
                _serviceLogger->debug("Stopping services attached to time slot N°{}", slot);
                
                if (it_vec->size() == 1) {
                    const Id id = it_vec->front();
                    stopService_internal(id);
                } else {
                    
                    std::vector<std::future<void> > futures;
                    for (std::vector<Id>::const_iterator it = it_vec->begin(); it != it_vec->end(); it++) {
                        const Id id = *it;
                        
                        std::launch policy = globalPolicy;
                        if (context._options.find(id)->second & Service::DefferedStartStop) {
                            policy = std::launch::deferred;
                        }
                        futures.push_back(std::async(policy, &Handler::stopService_internal, this, id));
                    }
                    
                    for (std::future<void> &future : futures) {
                        future.get();
                    }
                }
                
                _serviceLogger->debug("Services attached to time slot N°{} are now stopped", slot);
                
                if (slot == 0) {
                    break;
                }
                
                slot--;
            }
            
            _graph.reset();
        }
        
        HandlerState Handler::getState() {
            
            std::lock_guard<std::mutex> lock(_mutex);
            return _state;
        }
        
        HandlerStatus Handler::getStatus() {
            
            HandlerStatus result = {
                getState(), _registryPtr->getSericesStatus()
            };
            
            return result;
        }
        
        ServiceBase::Ptr Handler::subscribe_internal(Container container) {
            
            container._ptr->initialize(container.getId(), _registryPtr);
            _registryPtr->subscribe(container);
            
            return container._ptr;
        }
        
        ServiceBase::Ptr Handler::getServicePtr_internal(const Id &id) {
            return _registryPtr->getServicePtr(id);
        }
        
        void Handler::startService_internal(const Id &id, const FactoryBase::Ptr factoryPtr) {
            
            ServiceBase::Ptr servicePtr = getServicePtr_internal(id);
            if (servicePtr == nullptr) {
                
                Container newContainer(factoryPtr);
                
                _serviceLogger->debug("Automatically creating (dependency) service {}",
                                      newContainer.formatServiceInfo());
                
                servicePtr = subscribe_internal(newContainer);
            }
            
            _registryPtr->startService(id);
        }
        
        void Handler::stopService_internal(const Id &id) {
            _registryPtr->stopService(id);
        }
        
    }
}
