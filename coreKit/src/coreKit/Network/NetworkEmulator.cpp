//
//  NetworkEmulator.cpp
//  coreKit
//
//  Created by Pierre Pelé on 23/10/2017.
//

#include "NetworkEmulator.hpp"

#include <cmath>

#include <boost/asio/deadline_timer.hpp>

namespace coreKit { namespace Network {
    
    // Network configs
    
    static std::map<NetworkType, NetworkEmulator::Config> networkConfigs = {
        { NetworkType::Perfect  , { 0       , { 0       , 0     } } },
        { NetworkType::Bad      , { 0.3     , { 100000  , 10000 } } },
        { NetworkType::HSDPA_3G , { 0.15    , { 250000  , 25000 } } },
        { NetworkType::Edge     , { 0.15    , { 300000  , 30000 } } },
        { NetworkType::GPRS     , { 0.2     , { 500000  , 50000 } } },
        { NetworkType::Wifi     , { 0.2     , { 40000   , 4000  } } }
    };
    
    // NetworkEmulator
    
    NetworkEmulator::NetworkEmulator(boost::asio::io_service &ioService,
                                     const Config &config) :
    
    _strand     (ioService),
    _config     (config),
    _state      (Stopped)
    
    {
        if (config._dropRate < 0 ||
            config._dropRate > 1) {
            throw std::invalid_argument("Invalid drop rate");
        }
    }
    
    NetworkEmulator::NetworkEmulator(boost::asio::io_service &ioService,
                                     NetworkType networkType) :
    
    NetworkEmulator(ioService, networkConfigs[networkType])
    
    { }
    
    void NetworkEmulator::init(const Adapter::Callbacks &callbacks) {
        
        auto ptr = shared_from_this();
        _strand.dispatch([ptr, callbacks]() {
            
            // Note : Call by worker
            
            // Check state
            if (ptr->_state != Stopped) {
                return;
            }
            
            // Save user callbacks
            ptr->_callbacks = callbacks;
            
            // Update state
            ptr->_state = Started;
            
        });
    }
    
    void NetworkEmulator::cancel() {
        auto ptr = shared_from_this();
        _strand.dispatch([ptr]() {
            ptr->disconnect();
        });
    }
    
    void NetworkEmulator::disconnect() {
        
        // Note : Call by worker
        
        // Check state
        if (_state != Started) {
            return;
        }
        
        // Cancel tasks
        _disconnection_Signal();
        
        // Unereference user callbacks
        _callbacks = Callbacks();
        
        // Update state
        _state = Stopped;
    }
    
    void NetworkEmulator::handleIncommingData(const Buffer &buffer,
                                              const WriteCallback &writeCallback) {
        _strand.dispatch(std::bind(&NetworkEmulator::handleData, shared_from_this(),
                                   Incomming,
                                   buffer,
                                   writeCallback));
    }
    
    void NetworkEmulator::handleOutgoingData(const Buffer &buffer,
                                             const WriteCallback &writeCallback) {
        _strand.dispatch(std::bind(&NetworkEmulator::handleData, shared_from_this(),
                                   Outgoing,
                                   buffer,
                                   writeCallback));
    }
    
    void NetworkEmulator::handleData(const Direction direction,
                                     const Buffer &buffer,
                                     const WriteCallback &writeCallback) {
        
        // Note : call by worker
        
        if (_state != Started) {
            auto error = std::make_exception_ptr(std::runtime_error(std::string("Method [") +
                                                                    __PRETTY_FUNCTION__ +
                                                                    "] : Invalid state (" +
                                                                    std::to_string(_state) + ")"));
            
            if (writeCallback) {
                writeCallback(error);
            }
            
            return;
        }
        
        // Should we drop the packet ?
        
        if (genDrop()) {
            if (writeCallback) {
                writeCallback(nullptr);
            }
            return;
        }
        
        // Our task container
        
        struct Task {
            
            Task(boost::asio::io_service &ioService) :
            
            _finished(false), _timer(ioService) { }
            
            void clean() {
                
                _timer.cancel();
                _connections.clear();
                
            }
            
            // Attributes
            
            bool _finished;
            boost::asio::deadline_timer _timer;
            std::vector<boost::signals2::scoped_connection> _connections;
            
        };
        
        auto taskPtr = std::make_shared<Task>(_strand.get_io_service());
        
        // Disconnection callback
        
        auto onDisconnection = [taskPtr]() {
            
            // Note : Call by worker
            
            if (!taskPtr->_finished) {
                
                taskPtr->_finished = true;
                taskPtr->clean();
            }
        };
        
        // Subscribe disconnection callbacks
        
        taskPtr->_connections.push_back(_disconnection_Signal.connect(onDisconnection));
        
        // Copy buffer
        
        auto bufferSize = boost::asio::buffer_size(buffer);
        const auto bufferCpy = std::shared_ptr<uint8_t>(new uint8_t[bufferSize], std::default_delete<uint8_t[]>());
        boost::asio::buffer_copy(boost::asio::mutable_buffer(bufferCpy.get(), bufferSize), buffer);
        
        if (writeCallback) {
            writeCallback(nullptr);
        }
        
        // Timer callback
        
        auto ptr = shared_from_this();
        auto timerCallback = [ptr, taskPtr, direction, bufferCpy, bufferSize](const boost::system::error_code &error) {
            
            // Note : Call by worker
            
            if (!error && !taskPtr->_finished) {
                
                auto buffer = Buffer(bufferCpy.get(), bufferSize);
                auto writeCallback = [bufferCpy](const std::exception_ptr) { };
                
                if (direction == Outgoing) {
                    ptr->_callbacks._onOutgoingData(buffer, writeCallback);
                } else {
                    ptr->_callbacks._onIncommingMessage(buffer, writeCallback);
                }
                
                taskPtr->_finished = true;
                taskPtr->clean();
            }
        };
        
        if (_config._latency._mean == 0) {
            
            // Process the packet immediately
            
            timerCallback(boost::system::error_code());
            
        } else {
            
            // Start timer
            
            taskPtr->_timer.expires_from_now(boost::posix_time::microseconds(genLatency()));
            taskPtr->_timer.async_wait(_strand.wrap(timerCallback));
        }
        
    }
    
    // Random number generator
    
    uint64_t NetworkEmulator::genLatency(const Distribution::param_type &params) {
        
        static std::random_device   rd;
        static std::mt19937         gen(rd());
        static Distribution         distribution(params);
        
        int64_t result;
        while (true) {
            result = std::round(distribution(gen));
            if (result >= 0 && result <= 2 * params.mean()) {
                break;
            }
        }
        
        return result;
    }
    
    uint64_t NetworkEmulator::genLatency() {
        return genLatency(Distribution::param_type(_config._latency._mean, _config._latency._stddev));
    }
    
    bool NetworkEmulator::genDrop(double dropRate) {
        
        static std::knuth_b randEngine;
        
        std::bernoulli_distribution distribution(dropRate);
        return distribution(randEngine);
    }
    
    bool NetworkEmulator::genDrop() {
        return genDrop(_config._dropRate);
    }
    
} }
