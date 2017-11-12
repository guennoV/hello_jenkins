//
//  StackTrace.hpp
//  uavia-software-next
//
//  Created by Pierre Pelé on 17/03/17.
//
//

#pragma once

#include <string>

namespace coreKit {
    
    // Add StackTrace ability
    
    void handleStackTrace();
    
    // Get the backtrace at the current point of execution
    
    std::string getBackTrace();
    
}
