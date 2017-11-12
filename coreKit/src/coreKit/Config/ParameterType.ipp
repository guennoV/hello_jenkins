//
//  ParameterType.ipp
//  uavia-software-next
//
//  Created by Pierre Pelé on 21/10/2016.
//
//

#pragma once

#include <typeinfo>

#include "ParameterType.hpp"

namespace coreKit {
    
    // ParameterType
    
    template<class F, class... Args>
    ParameterType::Entry ParameterType::addEntry(F&& f, Args&&... args) {
        
        using return_type = typename std::result_of<F(const YAML::Node &, Args...)>::type;
        return std::make_pair(typeid(return_type).hash_code(), std::make_shared<ParameterTypeExt<return_type> >(std::bind(std::forward<F>(f), std::placeholders::_1, std::forward<Args>(args)...)));
    }
    
    template <typename Type>
    ParameterTypeExt<Type>::ParameterTypeExt(const std::function<Type(const YAML::Node &)> &parser) : _parser(parser) { }
    
    template <typename Type>
    Type ParameterTypeExt<Type>::convert(const YAML::Node &node) {
        return _parser(node);
    }
    
}
