//
//  Adapter.hpp
//  coreKit
//
//  Created by Pierre Pelé on 23/10/2017.
//

#pragma once

#include <memory>

#include "Common.hpp"

namespace coreKit { namespace Network {
    
    // Adapter
    
    class Adapter {
        
    public:
        
        // Declarations
        
        using Ptr = std::shared_ptr<Adapter>;
        
        // Callbacks
        
        struct Callbacks {
            
            // Function to be called to handle incomming data
            
            OnIncommingData     _onIncommingMessage;
            
            // Function to be called to handle outgoing data
            
            OnOutgoingData      _onOutgoingData;
            
        };
        
        // Initialize / Cancel
        
        virtual void init(const Adapter::Callbacks &callbacks) = 0;
        virtual void cancel() = 0;
        
        // Adapter interface
        
        virtual void handleIncommingData(const Buffer &buffer,
                                         const WriteCallback &writeCallback) = 0;
        
        virtual void handleOutgoingData(const Buffer &buffer,
                                        const WriteCallback &writeCallback) = 0;
        
    };
    
    // DummyAdapter
    
    class DummyAdapter : public Adapter {
        
    public:
        
        // Initialize / Cancel
        
        void init(const Adapter::Callbacks &callbacks) override;
        void cancel() override;
        
        // Adapter interface
        
        void handleIncommingData(const Buffer &buffer,
                                 const WriteCallback &writeCallback) override;
        
        void handleOutgoingData(const Buffer &buffer,
                                const WriteCallback &writeCallback) override;
        
    private:
        
        // Private declarations
        
        enum State {
            
            Stopped = 0,
            Started = 1
            
        };
        
        // Attributes
        
        // User callbacks
        
        Adapter::Callbacks _callbacks;
        
    };
    
} }
