//
//  Dependency.cpp
//  embedded-software
//
//  Created by Pierre Pelé on 05/04/17.
//
//

#include "Dependency.hpp"
#include "Registry.hpp"

namespace coreKit {
    
    namespace Service {
        
        // DependencyContext
        
        std::string DependencyContext::toString() const {
            
            std::string result("Services :");
            
            if (_factories.size() == 0) {
                result += "\n\t--- No service ---";
            } else {
                for (std::map<Id, FactoryBase::Ptr>::const_iterator it = _factories.begin(); it != _factories.end(); it++) {
                    result += "\n\t" + formatServiceInfo(it->first);
                }
            }
            
            result += "\n\nDependencies :";
            
            if (_edges.size() == 0) {
                result += "\n\t--- No dependency ---";
            } else {
                for (Edge edge : _edges) {
                    result += "\n\tService '" + std::to_string(edge.first) + "' depends on '" + std::to_string(edge.second) + "'";
                }
            }
            
            return result;
        }
        
        std::string DependencyContext::formatServiceInfo(const Id &id) const {
            
            std::map<Id, Name>::const_iterator it = _names.find(id);
            
            if (it != _names.end()) {
                return Registry::formatServiceInfo(it->second, id);
            } else {
                throw std::runtime_error(std::string("Method [") +
                                         __PRETTY_FUNCTION__ +
                                         "] : Unknown service with ID '" +
                                         std::to_string(id) + "'");
            }
        }
        
        // DependencyGraph
        
        std::string DependencyGraph::toString() const {
            
            std::string result = _context.toString();
            
            result += "\n\nStart order :";
            if (_startOrder.size() == 0) {
                result += "\n\t--- Nothing to start ---";
            } else {
                
                int i = 0;
                for (std::vector<Id> vec : _startOrder) {
                    result += "\n\tTime slot N°" + std::to_string(i);
                    for (Id id : vec) {
                        result += "\n\t\t" + std::to_string(id);
                    }
                    i++;
                }
            }
            
            return result;
        }
    }
}
