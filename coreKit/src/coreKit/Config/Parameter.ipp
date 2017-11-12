//
//  Parameter.ipp
//  droneapi-xcode
//
//  Created by Pierre Pelé on 16/10/16.
//
//

#pragma once

#include <typeinfo>

#include "Parameter.hpp"

namespace coreKit {
    
    // ParameterExt
    
    template <typename Type>
    ParameterExt<Type>::ParameterExt(const std::string &name,
                                     const std::string &documentation,
                                     const Type &defaultValue) :
    
    Parameter       (name, documentation),
    
    _defaultValue   (defaultValue),
    _value          (defaultValue)
    
    { }
    
    template <typename Type>
    size_t ParameterExt<Type>::getHashCode() const {
        return typeid(Type).hash_code();
    }
    
    template <typename Type>
    Type ParameterExt<Type>::get() const {
        return _value;
    }
    
    template <typename Type>
    Type ParameterExt<Type>::getDefault() const {
        return _defaultValue;
    }
    
    template <typename Type>
    void ParameterExt<Type>::set(const YAML::Node &node) {
        set(convert(node));
    }
    
    template <typename Type>
    void ParameterExt<Type>::set(const Type &value) {
        _value = value;
    }
    
    template <typename Type>
    std::string ParameterExt<Type>::get_generic() const {
        
        YAML::Node node;
        node.push_back(_value);
        
        return node.begin()->as<std::string>();
    }
    
    template <typename Type>
    std::string ParameterExt<Type>::getDefaultValue_generic() const {
        
        YAML::Node node;
        node.push_back(_defaultValue);
        
        return node.begin()->as<std::string>();
    }
    
    template <typename Type>
    Type ParameterExt<Type>::convert(const YAML::Node &node) {
        
        // return std::dynamic_pointer_cast<ParameterTypeExt<Type> >(getParamType(getHashCode()))->convert(node);
        return node.as<Type>();
    }
    
}
