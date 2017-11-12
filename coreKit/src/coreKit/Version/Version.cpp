//
//  Version.cpp
//  uavia-embedded
//
//  Created by Pierre Pelé on 13/08/2017.
//
//

#include "Version.hpp"

#include <sstream>

namespace coreKit {
    
    // VersionInfo
    
    VersionInfo::VersionInfo(unsigned int major,
                             unsigned int minor,
                             unsigned int revision,
                             const std::string &release) :
    
    _major          (major),
    _minor          (minor),
    _revision       (revision),
    _release        (release)
    
    { }
    
    std::string VersionInfo::getVersionNumber() const {
        
        std::stringstream stream;
        stream << _major << "." << _minor << "." << _revision;
        if (_release.size() > 0) {
            stream << "-"  << _release;
        }
        
        return stream.str();
    }
    
    // BuildInfo
    
    bool BuildInfo::isCrossCompilation() const {
        return _system._build._name.compare(_system._host._name);
    }
    
    namespace Version {
        
        // Handle
        
        Handle::Handle(const ObjectName &objectName,
                       const VersionInfo &versionInfo) : Handle(objectName,
                                                                versionInfo,
                                                                boost::optional<BuildInfo>()) { }
        Handle::Handle(const ObjectName &objectName,
                       const VersionInfo &versionInfo,
                       const BuildInfo &buildInfo) : Handle(objectName,
                                                            versionInfo,
                                                            boost::optional<BuildInfo>(buildInfo)) { }
        Handle::Handle(const ObjectName &objectName,
                       const VersionInfo &versionInfo,
                       const boost::optional<BuildInfo> &buildInfo) :
        
        _objectName     (objectName),
        _versionInfo    (versionInfo),
        _buildInfo      (buildInfo)
        
        {
            Handler::add(*this);
        }
        
        ObjectName Handle::getObjectName() const {
            return _objectName;
        }
        
        VersionInfo Handle::getVersionInfo() const {
            return _versionInfo;
        }
        
        bool Handle::hasBuildInfo() const {
            return _buildInfo.operator bool();
        }
        
        BuildInfo Handle::getBuildInfo() const {
            if (hasBuildInfo()) {
                return _buildInfo.get();
            } else {
                throw std::runtime_error("Build informations are not available for this object");
            }
        }
        
        std::string Handle::getVersionNumber() const {
            return _versionInfo.getVersionNumber();
        }
        
        // Handler
        
        void Handler::add(const Handle &handle) {
            getInstance()->add_internal(handle);
        }
        
        void Handler::add_internal(const Handle &handle) {
            
            auto objectName = handle.getObjectName();
            if (_handles.find(objectName) != _handles.end()) {
                throw std::runtime_error("An object with name '" + objectName + "' is already registered");
            }
            
            _handles.insert(std::make_pair(handle.getObjectName(), handle));
        }
        
    }
}
