//
//  Parameter.cpp
//  droneapi-xcode
//
//  Created by Pierre Pelé on 16/10/16.
//
//

#include "Parameter.hpp"
#include "Config.hpp"

namespace coreKit {
    
    // Parameter
    
    const std::string Parameter::_delimiter = ".";
    
    Parameter::Parameter(const std::string &name,
                         const std::string &documentation) :
    _name(name), _documentation(documentation) { }
    
    ParameterType::Ptr Parameter::getParamType(size_t hashCode) {
        return Config::getParamType(hashCode);
    }
    
    std::string Parameter::getName() const {
        return _name;
    }
    
    std::string Parameter::getDocumentation() const {
        return _documentation;
    }
    
    void Parameter::set_generic(const std::string &value) {
        return set(YAML::Load(value));
    }
    
    std::string Parameter::toString() {
        
        std::string result;
        
        result += "- Name : " + _name + "\n";
        result += "- Documentation : " + getDocumentation() + "\n";
        
        return result;
    }
    
    std::string Parameter::toString(const Parameter::Parents &parents,
                                    const std::string &delimiter) {
        
        std::string result;
        
        for (const auto &str : parents) {
            result += (result.size() > 0 ? delimiter : "") + str;
        }
        
        return result;
    }
    
    std::string Parameter::toString(const Parameter::Parents &parents) {
        return toString(parents, _delimiter);
    }
    
    std::string Parameter::toString(const Parameter::FullName &fullname) {
        return toString(fullname.first) + (fullname.first.size() > 0 ? _delimiter : "") + fullname.second;
    }
    
    Parameter::FullName Parameter::decodeParameterName(const std::string &fullname) {
        
        Parents parents;
        
        size_t pos = 0;
        std::string strCopy = fullname;
        while ((pos = strCopy.find(_delimiter)) != std::string::npos) {
            
            parents.push_back(strCopy.substr(0, pos));
            strCopy.erase(0, pos + _delimiter.length());
        }
        
        return std::make_pair(parents, strCopy);
    }
    
    // ParameterList
    
    ParameterList::ParameterList(const Parameter::Parents &name,
                                 const std::map<std::string, Parameter::Ptr> &parameters) :
    
    _name(name),
    _parameters(parameters)
    
    { }
    
    Parameter::Parents ParameterList::getName() const { return _name; }
    std::map<std::string, Parameter::Ptr> ParameterList::getParmeters() const { return _parameters; }
    
    Parameter::Ptr ParameterList::getParam(const std::string &name) const {
        
        std::map<std::string, Parameter::Ptr>::const_iterator it = _parameters.find(name);
        if (it != _parameters.end()) {
            return it->second;
        }
        
        throw std::invalid_argument("Invalid parameter name [" + name + "]");
    }
    
    void ParameterList::insert(const std::map<std::string, Parameter::Ptr> &parameters) {
        
        for (std::map<std::string, Parameter::Ptr>::const_iterator it = parameters.begin(); it != parameters.end(); it++) {
            if (_parameters.find(it->first) != _parameters.end()) {
                throw std::invalid_argument("Parameter with name [" + Parameter::toString(std::make_pair(getName(), it->first)) + "] already exists");
            } else {
                _parameters.insert(std::make_pair(it->first, it->second));
            }
        }
    }
    
    // ConfigList
    
    ConfigList::ConfigList(const Parameter::Parents &parents,
                           const std::vector<Parameter::Ptr> &parameters) {
        
        ParameterList parameterList(parents, convert(parents, parameters));
        Config::add(parameterList);
    }
    
    std::map<std::string, Parameter::Ptr> ConfigList::convert(const Parameter::Parents &parents,
                                                              const std::vector<Parameter::Ptr> &parameters) {
        
        std::map<std::string, Parameter::Ptr> result;
        for (auto &paramPtr : parameters) {
            
            if (result.find(paramPtr->getName()) != result.end()) {
                throw std::invalid_argument("Parameter with name [" + Parameter::toString(std::make_pair(parents, paramPtr->getName())) + "] already exists");
            } else {
                result.insert(std::make_pair(paramPtr->getName(), paramPtr));
            }
        }
        
        return result;
    }
    
}
