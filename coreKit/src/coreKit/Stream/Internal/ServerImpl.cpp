//
//  ServerImpl.cpp
//  uavia-embedded
//
//  Created by Pierre Pelé on 14/09/2017.
//
//

#include "ServerImpl.hpp"

#include <network/uri.hpp>

namespace coreKit {
    
    namespace Stream {
        
        // TcpServer
        
        TcpServer::TcpServer(boost::asio::io_service &ioService,
                             const boost::asio::ip::tcp::endpoint &localEndpoint) :
        
        ServerImpl(ioService, [localEndpoint](boost::asio::ip::tcp::acceptor &acceptor) {
            
            // Open acceptor
            acceptor.open(localEndpoint.protocol());
            
            // Set option
            acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
            
            // Bind
            acceptor.bind(localEndpoint);
            
            // Listen
            acceptor.listen();
            
        }) { }
        
        std::string TcpServer::getLocalUri_internal() {
            
            // Note : Call by worker
            
            std::stringstream result;
            result << "tcp://";
            result << getLocalEndpoint_internal();
            
            return result.str();
        }
        
        // UnixServer
        
        UnixServer::UnixServer(boost::asio::io_service &ioService,
                               const boost::asio::local::stream_protocol::endpoint &localEndpoint) :
        
        ServerImpl(ioService, [localEndpoint](boost::asio::local::stream_protocol::acceptor &acceptor) {
            
            // Remove previous binding
            ::unlink(localEndpoint.path().c_str());
            
            // Open acceptor
            acceptor.open(localEndpoint.protocol());
            
            // Bind
            acceptor.bind(localEndpoint);
            
            // Listen
            acceptor.listen();
            
        }) { }
        
        std::string UnixServer::getLocalUri_internal() {
            
            // Note : Call by worker
            
            std::stringstream result;
            result << "unix:";
            result << getLocalEndpoint_internal();
            
            return result.str();
        }
        
        // Helpers
        
        Server::Ptr makeServer(boost::asio::io_service &ioService,
                               const std::string &localUri) {
            
            network::uri uri(localUri);
            
            if (!uri.has_scheme()) {
                std::invalid_argument("URI must contain a scheme");
            }
            
            auto scheme = uri.scheme();
            if (scheme.compare("tcp") == 0) {
                
                boost::asio::ip::tcp::endpoint endpoint;
                
                if (uri.has_host()) {
                    
                    try {
                        endpoint.address(boost::asio::ip::address::from_string(uri.host().to_string()));
                    } catch (const boost::system::system_error &exception) {
                        std::rethrow_exception(std::make_exception_ptr(exception));
                    } catch (...) {
                        std::rethrow_exception(std::current_exception());
                    }
                    
                }
                
                if (uri.has_port()) {
                    endpoint.port(uri.port<unsigned short>());
                }
                
                return std::make_shared<TcpServer>(ioService, endpoint);
                
            } else if (scheme.compare("unix") == 0) {
                
                if (!uri.has_path()) {
                    std::invalid_argument("Remote URI must contain a path");
                }
                
                boost::asio::local::stream_protocol::endpoint endpoint(uri.path().to_string());
                return std::make_shared<UnixServer>(ioService, endpoint);
                
            } else {
                throw std::invalid_argument("Scheme '" + scheme.to_string() + "' is invalid");
            }
        }
        
    }
}
