//
//  Singleton.ipp
//  uavia-software-next
//
//  Created by Pierre Pelé on 28/12/16.
//
//

#pragma once

#include "Singleton.hpp"

namespace coreKit {
    
    // Singleton
    
    template <class T> Singleton<T>::Singleton() { }
    template <class T> template<class... Args> std::shared_ptr<T> Singleton<T>::getInstance(Args&&... args) {
        static std::shared_ptr<T> instance(new T(std::forward<Args>(args)...));
        return instance;
    }
    
}
