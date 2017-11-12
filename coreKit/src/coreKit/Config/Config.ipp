//
//  Config.ipp
//  uavia-software-next
//
//  Created by Pierre Pelé on 02/11/16.
//
//

#pragma once

#include "Config.hpp"

namespace coreKit {
    
    // Config
    
    template <typename Type>
    std::shared_ptr<ParameterExt<Type> > Config::getParam(const Parameter::FullName &fullName) {
        std::shared_ptr<ParameterExt<Type> > result = std::dynamic_pointer_cast<ParameterExt<Type> >(getInstance()->getParamBase(fullName));
        if (result) {
            return result;
        }
        
        throw std::invalid_argument("Invalid type for parameter with name [" + Parameter::toString(fullName) + "]");
    }
    
    template <typename Type>
    std::shared_ptr<ParameterExt<Type> > Config::getParam(const std::string &parameterName) {
        return getParam<Type>(Parameter::decodeParameterName(parameterName));
    }
    
    template <typename Type>
    Type Config::get(const Parameter::FullName &fullName) {
        return getParam<Type>(fullName)->get();
    }
    
    template <typename Type>
    Type Config::get(const std::string &parameterName) {
        return get<Type>(Parameter::decodeParameterName(parameterName));
    }
    
    template <typename Type>
    void Config::set(const Parameter::FullName &fullName, const Type &value) {
        return getParam<Type>(fullName)->set(value);
    }
    
    template <typename Type>
    void Config::set(const std::string &parameterName, const Type &value) {
        return set<Type>(Parameter::decodeParameterName(parameterName), value);
    }
    
}
