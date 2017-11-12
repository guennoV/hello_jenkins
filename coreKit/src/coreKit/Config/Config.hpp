//
//  Config.hpp
//  droneapi-xcode
//
//  Created by Pierre Pelé on 16/10/16.
//
//

#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <utility>

#include <coreKit/Utils/Singleton.hpp>

#include "Parameter.hpp"

namespace coreKit {
    
    struct AppConfig {
        
        // To String
        
        std::string toString();
        
        // Attributes
        
        std::map<Parameter::FullName, Parameter::Ptr> _config;
    };
    
    // Config
    
    class Config : private Singleton<Config> {
        
        friend class ::coreKit::Parameter;
        friend class ::coreKit::Singleton<Config>;
        
    public:
        
        // Parse configuration file
        
        static void parseYaml(const std::string &filename);
        
        // Parse environment
        
        static void parseEnv();
        
        // Add parameters
        
        static void add(const ParameterList &parameterList);
        
        // Access to params (type oriented)
        
        template <typename Type> static std::shared_ptr<ParameterExt<Type> > getParam(const Parameter::FullName &fullName);
        template <typename Type> static std::shared_ptr<ParameterExt<Type> > getParam(const std::string &parameterName);
        
        // Access to params (type insensitive)
        
        static Parameter::Ptr getParam_generic(const Parameter::FullName &fullName);
        static Parameter::Ptr getParam_generic(const std::string &parameterName);
        
        // Get / Set params (type oriented)
        
        template <typename Type> static Type get(const Parameter::FullName &fullName);
        template <typename Type> static Type get(const std::string &parameterName);
        
        template <typename Type> static void set(const Parameter::FullName &fullName, const Type &value);
        template <typename Type> static void set(const std::string &parameterName, const Type &value);
        
        // Get / Set params (type insensitive)
        
        static std::string get_generic(const Parameter::FullName &fullName);
        static std::string get_generic(const std::string &parameterName);
        
        static void set_generic(const Parameter::FullName &fullName, const std::string &value);
        static void set_generic(const std::string &parameterName, const std::string &value);
        
        // Get all config
        
        static AppConfig getAppConfig();
        
    private:
        
        // Init
        
        Config();
        
        // Parse configuration file
        
        void parseYaml(const YAML::Node config);
        void parseYaml(const YAML::Node config,
                       const Parameter::Parents &parents,
                       const std::string &parameterName = "");
        
        void parseEnv_internal();
        
        // Add parameter list
        
        void add_internal(const ParameterList &parameterList);
        
        // Get / Set params
        
        Parameter::Ptr getParamBase(const Parameter::FullName &fullName);
        template <typename Type> std::shared_ptr<ParameterExt<Type> > getParam_internal(const Parameter::FullName &fullName);
        ParameterType::Ptr getParamType_internal(size_t hashCode);
        std::string get_internal(const Parameter::FullName &fullName);
        void set_internal(const Parameter::FullName &fullName, const std::string &value);
        AppConfig getAppConfig_internal();
        
        // Others
        
        static ParameterType::Ptr getParamType(size_t hashCode);
        static std::pair<Parameter::Parents, ParameterList> makeEntry(const ParameterList &parameterList);
        
        // Attributes
        
        std::mutex _mutex;
        
        std::map<Parameter::Parents, ParameterList>    _fullParameterList;
        std::vector<ParameterType::Map>                _parameterTypes;
        
    };
    
}

#include "Config.ipp"
