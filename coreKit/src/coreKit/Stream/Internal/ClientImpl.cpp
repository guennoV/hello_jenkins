//
//  ClientImpl.cpp
//  uavia-embedded
//
//  Created by Pierre Pelé on 14/09/2017.
//
//

#include "ClientImpl.hpp"

#include <boost/lexical_cast.hpp>
#include <network/uri.hpp>

namespace coreKit {
    
    namespace Stream  {
        
        // TcpClient
        
        TcpClient::TcpClient(boost::asio::io_service &ioService,
                             const boost::asio::ip::tcp::endpoint &remoteEndpoint) :
        
        ClientImpl(ioService, [this, remoteEndpoint](boost::asio::ip::tcp::socket &socket) {
            
            // Open socket
            socket.open(remoteEndpoint.protocol());
            
            return remoteEndpoint;
        })
        
        { }
        
        // UnixClient
        
        UnixClient::UnixClient(boost::asio::io_service &ioService,
                               const boost::asio::local::stream_protocol::endpoint &remoteEndpoint) :
        
        ClientImpl(ioService, [remoteEndpoint](boost::asio::local::stream_protocol::socket&) {
            return remoteEndpoint;
        })
        
        { }
        
        // Helpers
        
        Client::Ptr makeClient(boost::asio::io_service &ioService,
                               const std::string &remoteUri) {
            
            network::uri uri(remoteUri);
            
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
                
                return std::make_shared<TcpClient>(ioService, endpoint);
                
            } else if (scheme.compare("unix") == 0) {
                
                if (!uri.has_path()) {
                    std::invalid_argument("Remote URI must contain a path");
                }
                
                boost::asio::local::stream_protocol::endpoint endpoint(uri.path().to_string());
                return std::make_shared<UnixClient>(ioService, endpoint);
                
            } else {
                throw std::invalid_argument("Scheme '" + scheme.to_string() + "' is invalid");
            }
        }
        
    }
}
