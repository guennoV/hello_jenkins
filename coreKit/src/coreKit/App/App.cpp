//
//  App.cpp
//  uavia-software-next
//
//  Created by Pierre Pelé on 02/01/17.
//
//

#include "App.hpp"
#include "CoreAPI.hpp"

namespace coreKit {
    
    // App
    
    App::App(const Service::HandlerCallbacks &callbacks) :
    
    _impl(std::make_shared<Application::CoreAPI>(callbacks))
    
    { }
    
    App::~App() {
        _impl->stop();
    }
    
    Application::CoreAPI::Ptr App::operator->() const {
        return _impl;
    }
    
}
