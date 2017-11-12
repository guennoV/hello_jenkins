//
//  Config.cpp
//  droneapi-xcode
//
//  Created by Pierre Pelé on 16/10/16.
//
//

#include "Config.hpp"

#include <memory>
#include <cstdlib>      // std::getenv

#include <boost/filesystem.hpp>
#include <yaml-cpp/yaml.h>

#include <coreKit/Log/Logger.hpp>

namespace coreKit {
    
    // Our logger
    
    Logger configLogger( { "coreKit", "Config" } );
    
    // Helpers
    
    template <typename Type>
    static ParameterType::Entry standardConversion() {
        return ParameterType::addEntry([](const YAML::Node &node) { return node.as<Type>(); });
    }
    
    const ParameterType::Map _standardTypes = {
        
        //        standardConversion<bool>              (),
        //        standardConversion<int>               (),
        //        standardConversion<long>              (),
        //        standardConversion<unsigned int>      (),
        //        standardConversion<unsigned long long>(),
        //        standardConversion<unsigned short>    (),
        //        standardConversion<std::string>       ()
        
    };
    
    // Config
    
    Config::Config() :
    
    _parameterTypes( {
        
        // Here are all parameter types
        
        // _standardTypes
        
    })
    
    { }
    
    void Config::parseYaml(const std::string &filename) {
        return getInstance()->parseYaml(YAML::LoadFile(filename));
    }
    
    void Config::parseYaml(const YAML::Node node) {
        
        std::vector<std::string> parents;
        
        if (node.IsMap()) {
            parseYaml(node, parents);
        } else if (!node.IsNull()) {
            throw std::runtime_error("YAML File must have a map structure");
        }
    }
    
    void Config::parseYaml(const YAML::Node node,
                           const Parameter::Parents &parents,
                           const std::string &parameterName) {
        
        if (node.IsMap()) {
            
            Parameter::Parents newParents = parents;
            if (parameterName.size() > 0) {
                newParents.push_back(parameterName);
            }
            
            for (YAML::const_iterator it = node.begin(); it != node.end(); ++it) {
                parseYaml(it->second, newParents, it->first.as<std::string>());
            }
            
        } else {
            
            const std::string paramName = Parameter::toString(std::make_pair(parents, parameterName));
            
            configLogger->trace("Method [{}] | Handling parameter [{}]",
                                __FUNCTION__ ,paramName);
            
            Parameter::Ptr param = getParamBase({ parents, parameterName });
            
            configLogger->debug("Method [{}] | Overriding parameter [{}] | Old value [{}] | New value [{}]",
                                __FUNCTION__,
                                paramName,
                                param->get_generic(),
                                node.as<std::string>());
            
            param->set(node);
        }
    }
    
    void Config::parseEnv() {
        return getInstance()->parseEnv_internal();
    }
    
    void Config::parseEnv_internal() {
        
        std::lock_guard<std::mutex> lock(_mutex);
        
        for (const auto &parameterList : _fullParameterList) {
            
            const std::string parents = Parameter::toString(parameterList.second.getName(), "_");
            for (const auto &param : parameterList.second.getParmeters()) {
                
                const std::string fullName = parents + "_" + param.first;
                const char* value = std::getenv(fullName.c_str());
                
                if (value != NULL) {
                    
                    const std::string paramName = Parameter::toString(std::make_pair(parameterList.second.getName(), param.first));
                    
                    configLogger->debug("Method [{}] | Overriding parameter [{}] | Old value [{}] | New value[{}]",
                                        __FUNCTION__,
                                        paramName,
                                        param.second->get_generic(),
                                        value);
                    
                    param.second->set_generic(value);
                }
            }
        }
    }
    
    void Config::add(const ParameterList &parameterList) {
        return getInstance()->add_internal(parameterList);
    }
    
    Parameter::Ptr Config::getParam_generic(const Parameter::FullName &fullName) {
        return getInstance()->getParamBase(fullName);
    }
    
    Parameter::Ptr Config::getParam_generic(const std::string &parameterName) {
        return getParam_generic(Parameter::decodeParameterName(parameterName));
    }
    
    std::string Config::get_generic(const Parameter::FullName &fullName) {
        return getInstance()->get_internal(fullName);
    }
    
    std::string Config::get_generic(const std::string &parameterName) {
        return get_generic(Parameter::decodeParameterName(parameterName));
    }
    
    void Config::set_generic(const Parameter::FullName &fullName, const std::string &value) {
        return getInstance()->set_internal(fullName, value);
    }
    
    void Config::set_generic(const std::string &parameterName, const std::string &value) {
        return set_generic(Parameter::decodeParameterName(parameterName), value);
    }
    
    AppConfig Config::getAppConfig() {
        return getInstance()->getAppConfig_internal();
    }
    
    std::string AppConfig::toString() {
        
        std::string output = "Application Config :\n";
        
        for (const auto &parameter : _config) {
            
            const auto param = parameter.second;
            const std::string fullname = Parameter::toString(parameter.first);
            
            output += "\t- " + fullname + " : " + param->get_generic() + "\n";
        }
        
        return output;
    }
    
    void Config::add_internal(const ParameterList &parameterList) {
        
        std::lock_guard<std::mutex> lock(_mutex);
        
        std::map<Parameter::Parents, ParameterList>::iterator it = _fullParameterList.find(parameterList.getName());
        if (it != _fullParameterList.end()) {
            it->second.insert(parameterList.getParmeters());
        } else {
            _fullParameterList.insert(std::make_pair(parameterList.getName(), parameterList));
        }
    }
    
    Parameter::Ptr Config::getParamBase(const Parameter::FullName &fullName) {
        
        std::lock_guard<std::mutex> lock(_mutex);
        
        std::map<Parameter::Parents, ParameterList>::const_iterator it = _fullParameterList.find(fullName.first);
        if (it != _fullParameterList.end()) {
            return it->second.getParam(fullName.second);
        }
        
        throw std::invalid_argument("Invalid parameter list with name : [" + Parameter::toString(fullName.first) + "]");
    }
    
    ParameterType::Ptr Config::getParamType_internal(size_t hashCode) {
        
        for (std::vector<ParameterType::Map>::const_iterator it_1 = _parameterTypes.begin(); it_1 != _parameterTypes.end(); it_1++) {
            ParameterType::Map::const_iterator it_2 = it_1->find(hashCode);
            if (it_2 != it_1->end()) {
                return it_2->second;
            }
        }
        
        throw std::invalid_argument("Invalid parameter type : [" + std::to_string(hashCode) + "]");
    }
    
    std::string Config::get_internal(const Parameter::FullName &fullName) {
        return getParamBase(fullName)->get_generic();
    }
    
    void Config::set_internal(const Parameter::FullName &fullName, const std::string &value) {
        return getParamBase(fullName)->set_generic(value);
    }
    
    AppConfig Config::getAppConfig_internal() {
        
        AppConfig appConfig;
        
        std::lock_guard<std::mutex> lock(_mutex);
        
        for (const auto &parameterList : _fullParameterList) {
            
            const auto parents = parameterList.second.getName();
            for (const auto &param : parameterList.second.getParmeters()) {
                
                Parameter::FullName fullname = { parents, param.second->getName() };
                appConfig._config.insert(std::make_pair(fullname, param.second));
            }
        }
        
        return appConfig;
    }
    
    ParameterType::Ptr Config::getParamType(size_t hashCode) {
        return getInstance()->getParamType_internal(hashCode);
    }
    
    std::pair<Parameter::Parents, ParameterList> Config::makeEntry(const ParameterList &parameterList) {
        return std::make_pair(parameterList.getName(), parameterList);
    }
    
}
