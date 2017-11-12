//
//  Singleton.hpp
//  uavia-software-next
//
//  Created by Pierre Pelé on 28/12/16.
//
//

#pragma once

#include <memory>

namespace coreKit {
    
    // Singleton
    
    template <class T> class Singleton {
    public:
        /* Non-copyable.*/
        Singleton(const Singleton&) = delete;
        Singleton & operator=(const Singleton&) = delete;
    protected:
        Singleton();
        template <class... Args> static std::shared_ptr<T> getInstance(Args&&... args);
    };
    
}

#include "Singleton.ipp"
