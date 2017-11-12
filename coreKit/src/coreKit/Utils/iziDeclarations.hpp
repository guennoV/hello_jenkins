//
//  iziDeclarations.hpp
//  uavia-software-next
//
//  Created by Mathieu Corti on 03/04/17.
//
//

#pragma once

#include <string>

namespace coreKit {
    
    // This will check if th egiven path is izi compliant
    
    bool validateIziRoute(const std::string &str);
    bool validateIziUrl(const std::string &str);
    
}
