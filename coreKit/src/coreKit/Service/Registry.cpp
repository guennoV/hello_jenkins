//
//  Registry.cpp
//  uavia-software-next
//
//  Created by Pierre Pelé on 05/01/17.
//
//

#include "Registry.hpp"

#include <algorithm>
#include <iterator>
#include <map>
#include <string>
#include <vector>
#include <utility>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/visitors.hpp>

#include "Declarations.hpp"

namespace coreKit {
    
    namespace Service {
        
        // Prototypes
        
        static DependencyContext buildDependencyContext(const std::map<Id, Container> &subscribedServices);
        static DependencyGraph sortServices(const DependencyContext &context);
        
        // CycleDetector
        
        struct CycleDetector : public boost::dfs_visitor<> {
        public:
            CycleDetector(bool &hasCycle) : _hasCycle(hasCycle) { }
            template <class Edge, class Graph> void back_edge(Edge, Graph&) { _hasCycle = true; }
        protected:
            bool& _hasCycle;
        };
        
        // Service
        
        Container::Container(ServiceBase::Ptr ptr, const Handle::Ptr handle) : _ptr(ptr), _handle(handle) {
            
            if (!_ptr) {
                throw std::runtime_error("Service is undefined");
            }
            
            if (!_handle) {
                throw std::runtime_error("Service handle is undefined");
            }
        }
        
        Container::Container(const FactoryBase::Ptr factory) : Container(factory->buildService(), factory->buildHandle()) { }
        
        Id Container::getId() const { return _handle->getId(); }
        Name Container::getName() const { return _handle->getName(); }
        Version Container::getVersion() const { return _handle->getVersion(); }
        
        std::string Container::formatServiceInfo() const {
            return Registry::formatServiceInfo(getName(), getId());
        }
        
        // Registry
        
        Registry::Registry(const std::function<void(const std::string serviceName,
                                                    const std::string &reason)> &onAppCloseRequest) : _onAppCloseRequest(onAppCloseRequest) { }
        
        void Registry::clear() {
            
            boost::unique_lock<boost::shared_mutex> lock(_mutex); // Write access
            
            for (std::map<Id, Container>::const_iterator it = _containers.begin(); it != _containers.end(); it++) {
                it->second._ptr->deinitialize();
            }
            
            _containers.clear();
        }
        
        void Registry::subscribe(const Container &container) {
            
            const Id id = container.getId();
            boost::unique_lock<boost::shared_mutex> lock(_mutex); // Write access
            if (_containers.find(id) != _containers.end()) {
                throw std::runtime_error(std::string("Method [") +
                                         __PRETTY_FUNCTION__ +
                                         "] : A service with ID '" +
                                         std::to_string(id) +
                                         "' is already registered");
            }
            
            _serviceLogger->debug("Subscribing service {}", container.formatServiceInfo());
            
            _containers.insert(std::make_pair(id, container));
        }
        
        Name Registry::getServiceName(const Id &id) {
            
            boost::shared_lock<boost::shared_mutex> lock(_mutex); // Read access
            std::map<Id, Container>::const_iterator it = _containers.find(id);
            
            if (it != _containers.end()) {
                return it->second.getName();
            } else {
                throw std::runtime_error(std::string("Method [") +
                                         __PRETTY_FUNCTION__ +
                                         "] : Unknown service with ID '" +
                                         std::to_string(id) + "'");
            }
        }
        
        std::string Registry::formatServiceInfo(const Id &id) {
            
            boost::shared_lock<boost::shared_mutex> lock(_mutex); // Read access
            std::map<Id, Container>::const_iterator it = _containers.find(id);
            
            if (it != _containers.end()) {
                return formatServiceInfo(it->second.getName(), id);
            } else {
                throw std::runtime_error(std::string("Method [") +
                                         __PRETTY_FUNCTION__ +
                                         "] : Unknown service with ID '" +
                                         std::to_string(id) + "'");
            }
        }
        
        Version Registry::getServiceVersion(const Id &id) {
            
            boost::shared_lock<boost::shared_mutex> lock(_mutex); // Read access
            std::map<Id, Container>::const_iterator it = _containers.find(id);
            
            if (it != _containers.end()) {
                return it->second.getVersion();
            } else {
                throw std::runtime_error(std::string("Method [") +
                                         __PRETTY_FUNCTION__ +
                                         "] : Unknown service with ID '" +
                                         std::to_string(id) + "'");
            }
        }
        
        void Registry::startService(const Id &id) {
            
            std::map<Id, Container>::const_iterator it;
            
            {
                boost::shared_lock<boost::shared_mutex> lock(_mutex); // Read access
                it = _containers.find(id);
                
                if (it == _containers.end()) {
                    throw std::runtime_error(std::string("Method [") +
                                             __PRETTY_FUNCTION__ +
                                             "] : Unknown service with ID '" +
                                             std::to_string(id) + "'");
                }
            }
            
            auto serviceInfo = formatServiceInfo(it->second.getName(), id);
            
            _serviceLogger->debug("Starting service {}",
                                  serviceInfo);
            
            it->second._ptr->start();
            
            _serviceLogger->debug("Service {} is now started",
                                  serviceInfo);
        }
        
        // Stop service
        
        void Registry::stopService(const Id &id) {
            
            std::map<Id, Container>::const_iterator it;
            
            {
                boost::shared_lock<boost::shared_mutex> lock(_mutex); // Read access
                it = _containers.find(id);
                
                if (it == _containers.end()) {
                    return;
                }
            }
            
            auto serviceInfo = formatServiceInfo(it->second.getName(), id);
            
            _serviceLogger->debug("Stopping service {}",
                                  serviceInfo);
            
            it->second._ptr->stop();
            
            _serviceLogger->debug("Service {} is now stopped",
                                  serviceInfo);
        }
        
        std::vector<Status> Registry::getSericesStatus() {
            
            std::vector<Status> result;
            boost::shared_lock<boost::shared_mutex> lock(_mutex); // Read access
            
            for (std::map<Id, Container>::const_iterator it = _containers.begin(); it != _containers.end(); it++) {
                
                Status status = {
                    it->second.getName(),
                    it->second.getVersion(),
                    it->second._ptr->getState()
                };
                
                result.push_back(status);
            }
            
            return result;
        }
        
        void Registry::requestAppClosing(const Id &id,
                                         const std::string &reason) {
            
            const std::string serviceName = getServiceName(id);
            
            if (_onAppCloseRequest) {
                _onAppCloseRequest(serviceName, reason);
            }
        }
        
        ServiceBase::Ptr Registry::getServicePtr(const Id &id) {
            
            boost::shared_lock<boost::shared_mutex> lock(_mutex); // Read access
            std::map<Id, Container>::const_iterator it = _containers.find(id);
            if (it != _containers.end()) {
                return it->second._ptr;
            }
            
            return nullptr;
        }
        
        DependencyGraph Registry::buildDependencyGraph() {
            return sortServices(buildDependencyContext(_containers));
        }
        
        std::string Registry::formatServiceInfo(const Name &name, const Id &id) {
            
            std::string result = "[" + name + "] [ID : " + std::to_string(id) + "]";
            
            return result;
        }
        
        // Static methods
        
        static DependencyContext buildDependencyContext(const std::map<Id, Container> &subscribedServices) {
            
            DependencyContext context;
            
            for (std::map<Id, Container>::const_iterator it = subscribedServices.begin(); it != subscribedServices.end(); it++) {
                it->second._handle->buildDependencyContext(context);
            }
            
            return context;
        }
        
        static DependencyGraph sortServices(const DependencyContext &context) {
            
            using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS>;
            using Vertex = boost::graph_traits<Graph>::vertex_descriptor;
            
            std::vector<Id> vec; // vec[size_t] -> id
            for (std::map<Id, FactoryBase::Ptr>::const_iterator it = context._factories.begin(); it != context._factories.end(); it++) {
                vec.push_back(it->first);
            }
            
            std::map<Id, size_t> map; // map[id] -> size_t
            {
                size_t pos = 0;
                for (Id id : vec) {
                    map.insert(std::make_pair(id, pos));
                    pos++;
                }
            }
            
            std::vector<Edge> edges;
            for (Edge edge : context._edges) {
                edges.push_back(std::make_pair(map[edge.second], map[edge.first]));
            }
            
            const size_t verticeSize = vec.size();
            Graph graph(edges.begin(), edges.end(), verticeSize);
            
            {
                bool hasCycle = false;
                CycleDetector vis(hasCycle);
                boost::depth_first_search(graph, visitor(vis));
                
                if (hasCycle) {
                    throw std::runtime_error("One service has cyclic dependency");
                }
            }
            
            // Parallel ordering
            
            std::vector<int> time(verticeSize, 0);
            
            {
                using StartOrderList = std::list<Vertex>;
                
                StartOrderList startOrder;
                boost::topological_sort(graph, std::front_inserter(startOrder));
                
                for (StartOrderList::iterator it = startOrder.begin(); it != startOrder.end(); ++it) {
                    
                    // Walk through the in_edges an calculate the maximum time.
                    
                    if (boost::in_degree(*it, graph) > 0) {
                        
                        Graph::in_edge_iterator j, j_end;
                        int maxdist = 0;
                        
                        // Through the order from topological sort, we are sure that every
                        // time we are using here is already initialized.
                        
                        for (boost::tie(j, j_end) = boost::in_edges(*it, graph); j != j_end; ++j) {
                            maxdist = (std::max)(time[source(*j, graph)], maxdist);
                        }
                        
                        time[*it] = maxdist + 1;
                    }
                }
            }
            
            std::vector<std::vector<Id> > result;
            
            {
                size_t vecSize = 0;
                boost::graph_traits<Graph>::vertex_iterator it, it_end;
                
                for (boost::tie(it,it_end) = vertices(graph); it != it_end; ++it) {
                    
                    const size_t pos  = time[*it];
                    
                    if ((pos + 1) > vecSize) {
                        
                        const size_t diff = (pos + 1) - vecSize;
                        for (size_t i = 0; i < diff; i++) {
                            result.push_back(std::vector<Id>());
                            vecSize++;
                        }
                    }
                    
                    result[pos].push_back(vec[*it]);
                }
            }
            
            return { context, result };
        }
        
    }
}
