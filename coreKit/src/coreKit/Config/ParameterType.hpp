//
//  ParameterType.hpp
//  uavia-software-next
//
//  Created by Pierre Pelé on 21/10/2016.
//
//

#pragma once

#include <functional>
#include <map>
#include <memory>
#include <utility>
#include <vector>

#include <yaml-cpp/yaml.h>

namespace coreKit {
    
    // ParameterType
    
    class ParameterType {
        
    public:
        
        // Declarations
        
        using Ptr   = std::shared_ptr   <ParameterType  >;
        using Entry = std::pair         <size_t, Ptr    >;
        using Map   = std::map          <size_t, Ptr    >;
        
        // Init
        
        virtual ~ParameterType();
        
        template<class F, class... Args>
        static Entry addEntry(F&& f, Args&&... args);
        
    };
    
    // ParameterTypeExt
    
    template <typename Type>
    class ParameterTypeExt : public ParameterType {
        
    public:
        
        ParameterTypeExt(const std::function<Type(const YAML::Node &)> &parser);
        
        Type convert(const YAML::Node &node);
        
    private:
        
        const std::function<Type(const YAML::Node &)> _parser;
        
    };
}

#include "ParameterType.ipp"
