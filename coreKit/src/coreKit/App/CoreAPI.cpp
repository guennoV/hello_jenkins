//
//  CoreAPI.cpp
//  uavia-software-next
//
//  Created by Pierre Pelé on 02/01/17.
//
//

#include "CoreAPI.hpp"

#include <fstream>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>

#include "Declarations.hpp"

#include <coreKit/Config/Config.hpp>

#ifdef HAVE_CONFIG_H
#include <coreKit_config.h>
#endif

namespace coreKit {
    
    namespace Application {
        
        // CoreAPI
        
        void CoreAPI::start()   { return _handlerPtr->start(); }
        void CoreAPI::stop()    { return _handlerPtr->stop();  }
        
        CoreAPI::CoreAPI(const Service::HandlerCallbacks &callbacks) : _handlerPtr(std::make_shared<Service::Handler>(callbacks)) {
            
#ifdef COREKIT_BANNERFILE
            
            // Log the banner !
            
            std::ifstream bannerFile(COREKIT_BANNERFILE);
            if (bannerFile) {
                
                const std::string bannerStr((std::istreambuf_iterator<char>(bannerFile)), std::istreambuf_iterator<char>());
                bannerFile.close();
                
                _appLogger->info("\n\n{}\n", bannerStr);
            } else {
                _appLogger->warn("Cannot open coreKit banner file !");
            }
            
#endif
        }
    }
}
