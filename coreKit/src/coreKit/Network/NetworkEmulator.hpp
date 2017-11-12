//
//  NetworkEmulator.hpp
//  coreKit
//
//  Created by Pierre Pelé on 23/10/2017.
//

#pragma once

#include <stdint.h>

#include <memory>
#include <random>

#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/signals2/signal.hpp>

#include "Adapter.hpp"

namespace coreKit { namespace Network {
    
    // Declarations
    
    // NetworkType
    
    enum class NetworkType {
        
        Perfect     = 0,
        Bad         = 1,
        HSDPA_3G    = 2,
        Edge        = 3,
        GPRS        = 4,
        Wifi        = 5
    };
    
    // NetworkEmulator
    
    class NetworkEmulator :
    public Adapter,
    public std::enable_shared_from_this<NetworkEmulator> {
        
    public:
        
        // Public declarations
        
        // Config
        
        struct Config {
            
            // Attributes
            
            double          _dropRate;  // Drop rate in %1
            
            struct {
                
                uint64_t    _mean;      // Average latency in microseconds
                uint64_t    _stddev;    // Standard deviation in microseonds
                
            } _latency;
            
        };
        
        // Init
        
        NetworkEmulator(boost::asio::io_service &ioService,
                        const Config &config);
        
        NetworkEmulator(boost::asio::io_service &ioService,
                        NetworkType networkType);
        
        // Initialize / Cancel
        
        void init(const Adapter::Callbacks &callbacks) override;
        void cancel() override;
        
        // Adapter interface
        
        void handleIncommingData(const Buffer &buffer,
                                 const WriteCallback &writeCallback) override;
        
        void handleOutgoingData(const Buffer &buffer,
                                const WriteCallback &writeCallback) override;
        
    private:
        
        // Private declations
        
        using Latency = uint64_t;
        using Distribution = std::normal_distribution<double>;
        
        // PacketProperties
        
        struct PacketProperties {
            
            // Attributes
            
            // The latency associated to the packet
            // 0 means the packet should be drop
            
            Latency _latency;
            
        };
        
        // State
        
        enum State {
            
            Started     = 0,
            Stopped     = 1
            
        };
        
        // Direction
        
        enum Direction {
            
            Outgoing    = 0,
            Incomming   = 1
            
        };
        
        // Private methods
        
        // Disconnection handling
        
        void disconnect();
        
        // Adapter interface helper
        
        void handleData(Direction direction,
                        const Buffer &buffer,
                        const WriteCallback &writeCallback);
        
        // Random number generator
        
        // Latency generator
        
        static uint64_t genLatency(const Distribution::param_type &params);
        
        uint64_t genLatency();
        
        // Drop generator
        
        static bool genDrop(double dropRate);
        
        bool genDrop();
        
        // Attributes
        
        // Time handling
        
        boost::asio::strand _strand;
        
        // Our config
        
        const Config _config;
        
        // Active state
        
        State _state;
        
        // User callbacks
        
        Adapter::Callbacks _callbacks;
        
        // Disconnection handling
        
        boost::signals2::signal<void()>     _disconnection_Signal;
        boost::signals2::scoped_connection  _onDisonnection_Connection;
        
    };
    
} }
