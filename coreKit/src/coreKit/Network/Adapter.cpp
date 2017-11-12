//
//  Adapter.cpp
//  coreKit
//
//  Created by Pierre Pelé on 23/10/2017.
//

#include "Adapter.hpp"

namespace coreKit { namespace Network {
    
    // DummyAdapter
    
    void DummyAdapter::init(const Adapter::Callbacks &callbacks) {
        _callbacks = callbacks;
        
    }
    
    void DummyAdapter::cancel() {
        _callbacks = Callbacks();
    }
    
    void DummyAdapter::handleIncommingData(const Buffer &buffer,
                                           const WriteCallback &writeCallback) {
        _callbacks._onIncommingMessage(buffer, writeCallback);
    }
    
    void DummyAdapter::handleOutgoingData(const Buffer &buffer,
                                          const WriteCallback &writeCallback) {
        _callbacks._onOutgoingData(buffer, writeCallback);
    }
    
} }
