//
//  Version.hpp
//  uavia-embedded
//
//  Created by Pierre Pelé on 13/08/2017.
//
//

#pragma once

#include <map>
#include <string>

#include <boost/optional.hpp>

#include <coreKit/Utils/Singleton.hpp>

namespace coreKit {
    
    // VersionInfo
    
    struct VersionInfo {
        
        // Methods
        
        VersionInfo(unsigned int major,
                    unsigned int minor,
                    unsigned int revision,
                    const std::string &release);
        
        std::string getVersionNumber() const;
        
        // Attributes
        
        unsigned int        _major;
        unsigned int        _minor;
        unsigned int        _revision;
        
        std::string         _release;
        
    };
    
    // CompilerInfo
    
    struct CompilerInfo {
        
        // Attributes
        
        std::string         _name;
        std::string         _version;
        
    };
    
    // SystemInfo
    
    struct SystemInfo {
        
        // Attributes
        
        std::string         _name;
        std::string         _cpu;
        
    };
    
    // BuildInfo
    
    struct BuildInfo {
        
        // Methods
        
        bool isCrossCompilation() const;
        
        // Attributes
        
        struct {
            
            // C Compiler infos
            
            CompilerInfo    _cc;
            
            // C++ Compiler infos
            
            CompilerInfo    _cxx;
            
        } _compiler;
        
        struct {
            
            // Build system infos
            
            SystemInfo      _build;
            
            // Host system infos
            
            SystemInfo      _host;
            
        } _system;
        
    };
    
    namespace Version {
        
        // Declarations
        
        using ObjectName = std::string;
        
        // Handle
        
        class Handle {
            
        public:
            
            Handle(const ObjectName &objectName,
                   const VersionInfo &versionInfo);
            Handle(const ObjectName &objectName,
                   const VersionInfo &versionInfo,
                   const BuildInfo &buildInfo);
            
            ObjectName getObjectName() const;
            VersionInfo getVersionInfo() const;
            
            bool hasBuildInfo() const;
            BuildInfo getBuildInfo() const;
            
            std::string getVersionNumber() const;
            
        private:
            
            Handle(const ObjectName &objectName,
                   const VersionInfo &versionInfo,
                   const boost::optional<BuildInfo> &buildInfo);
            
            const ObjectName _objectName;
            const VersionInfo _versionInfo;
            const boost::optional<BuildInfo> _buildInfo;
            
        };
        
        // Handler
        
        class Handler : private Singleton<Handler> {
            friend class ::coreKit::Singleton<Handler>;
        public:
            static void add(const Handle &handle);
        private:
            void add_internal(const Handle &handle);
            std::map<ObjectName, Handle> _handles;
        };
        
    }
}
