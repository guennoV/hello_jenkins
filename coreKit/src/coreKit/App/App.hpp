//
//  App.hpp
//  uavia-software-next
//
//  Created by Pierre Pelé on 02/01/17.
//
//

#pragma once

#include "CoreAPI.hpp"

namespace coreKit {
    
    // App
    
    class App {
        
    public:
        
        // Init
        
        App(const Service::HandlerCallbacks &callbacks = Service::HandlerCallbacks());
        ~App();
        
        /* Non-copyable.*/
        App(const App&) = delete;
        App & operator=(const App&) = delete;
        
        Application::CoreAPI::Ptr operator->() const;
        
    private:
        
        // Attributes
        
        Application::CoreAPI::Ptr _impl;
    };
    
}
