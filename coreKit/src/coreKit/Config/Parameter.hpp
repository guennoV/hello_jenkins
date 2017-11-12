//
//  Parameter.hpp
//  droneapi-xcode
//
//  Created by Pierre Pelé on 16/10/16.
//
//

#pragma once

#include <map>
#include <memory>
#include <vector>
#include <utility>
#include <string>

#include <yaml-cpp/yaml.h>

#include "ParameterType.hpp"

namespace coreKit {
    
    // Opaque declarations
    
    //class Config;
    
    // Parameter
    
    class Parameter {
        
    public:
        
        // Declarations
        
        using Ptr       = std::shared_ptr<Parameter>;
        using Parents   = std::vector<std::string>;
        using FullName  = std::pair<Parents, std::string>;
        
        static std::string toString(const Parents &parents);
        static std::string toString(const Parents &parents,
                                    const std::string &delimiter);
        static std::string toString(const FullName &fullname);
        
        static FullName decodeParameterName(const std::string &fullname);
        
    protected:
        
        // Init
        
        Parameter(const std::string &name,
                  const std::string &documentation);
        
        static ParameterType::Ptr getParamType(size_t hashCode);
        
    public:
        
        virtual size_t          getHashCode() const = 0;
        
        std::string             getName() const;
        std::string             getDocumentation() const;
        
        virtual void            set(const YAML::Node &node) = 0;
        void                    set_generic(const std::string &value);
        
        virtual std::string     get_generic() const = 0;
        virtual std::string     getDefaultValue_generic() const = 0;
        
        std::string             toString();
        
    private:
        
        // Attributes
        
        static const std::string _delimiter;
        
        const std::string   _name;
        const std::string   _documentation;
        
    };
    
    // ParameterExt
    
    template <typename Type>
    class ParameterExt : public Parameter {
        
    public:
        
        // Init
        
        ParameterExt(const std::string &name,
                     const std::string &documentation,
                     const Type &defaultValue);
        
        size_t  getHashCode() const override;
        
        Type    get() const;
        Type    getDefault() const;
        
        void set(const YAML::Node &node) override;
        void set(const Type &value);
        
        std::string get_generic() const override;
        std::string getDefaultValue_generic() const override;
        
    private:
        
        Type convert(const YAML::Node &node);
        
        // Attributes
        
        const Type  _defaultValue;
        Type        _value;
        
    };
    
    // Helpers
    
    template <class T, class... Args> Parameter::Ptr makeParam(Args&&... args) {
        return std::make_shared<ParameterExt<T> >(std::forward<Args>(args)...);
    }
    
    // ParameterList
    
    class ParameterList {
        
    public:
        
        // Init
        
        ParameterList(const Parameter::Parents &name,
                      const std::map<std::string, Parameter::Ptr> &parameters);
        
        Parameter::Parents getName() const;
        std::map<std::string, Parameter::Ptr> getParmeters() const;
        
        Parameter::Ptr getParam(const std::string &name) const;
        
        void insert(const std::map<std::string, Parameter::Ptr> &parameters);
        
    private:
        
        // Attributes
        
        const Parameter::Parents                    _name;
        std::map<std::string, Parameter::Ptr>       _parameters;
        
    };
    
    class ConfigList {
        
    public:
        ConfigList(const Parameter::Parents &parents, const std::vector<Parameter::Ptr> &parameters);
    private:
        static std::map<std::string, Parameter::Ptr> convert(const Parameter::Parents &parents,
                                                             const std::vector<Parameter::Ptr> &parameters);
        
    };
    
}

#include "Parameter.ipp"
