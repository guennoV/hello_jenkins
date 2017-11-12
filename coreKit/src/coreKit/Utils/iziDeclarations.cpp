//
//  iziDeclarations.cpp
//  uavia-software-next
//
//  Created by Mathieu Corti on 03/04/17.
//
//

#include <string>
#include <regex>

#ifndef UNAUTHORIZED_EXPR
#define UNAUTHORIZED_EXPR "[^*/]"
#endif

namespace coreKit {
    
    bool validateIziRoute(const std::string &str) {
        static const auto route = std::regex(std::string("^(?:\\*\\*(?!\\/\\*\\*)|\\*|") + UNAUTHORIZED_EXPR + "+)(?:\\/(?:\\*\\*(?!\\/\\*\\*)|\\*|" + UNAUTHORIZED_EXPR + "+))*$");
        return std::regex_match(str, route);
    }
    
    bool validateIziUrl(const std::string &str) {
        static const auto url = std::regex(std::string("^") + UNAUTHORIZED_EXPR + "+(?:\\/" + UNAUTHORIZED_EXPR + "+)*$");
        return std::regex_match(str, url);
    }
    
}

#undef UNAUTHORIZED_EXPR
