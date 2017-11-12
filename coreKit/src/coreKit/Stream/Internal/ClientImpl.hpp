//
//  ClientImpl.hpp
//  uavia-embedded
//
//  Created by Pierre Pelé on 14/09/2017.
//
//

#pragma once

#include <coreKit/Stream/Client.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/local/stream_protocol.hpp>

#include "SessionImpl.hpp"

namespace coreKit {
    
    namespace Stream {
        
        // ClientImpl
        
        template <class stream_type>
        class ClientImpl : public Client {
            
        protected:
            
            // Protected declarations
            
            using Endpoint = typename stream_type::endpoint;
            using Socket = typename stream_type::socket;
            using SessionType = SessionImpl<Socket>;
            
        public:
            
            // Public declarations
            
            using onConfigureSocket = std::function<Endpoint(Socket&)>;
            
            // Init
            
            ClientImpl(boost::asio::io_service &ioService,
                       const onConfigureSocket &onConfigureSocket);
            
        private:
            
            // Private methods
            
            // Create / Configure session
            
            void configure(const std::function<void(const boost::system::error_code&)> &handler) override;
            
            // Attributes
            
            // Function to be called to configure socket
            
            const onConfigureSocket _onConfigureSocket;
            
        };
        
        // TcpClient
        
        class TcpClient : public ClientImpl<boost::asio::ip::tcp> {
            
        public:
            
            // Init
            
            TcpClient(boost::asio::io_service &ioService,
                      const boost::asio::ip::tcp::endpoint &remoteEndpoint);
            
        };
        
        // UnixClient
        
        class UnixClient : public ClientImpl<boost::asio::local::stream_protocol> {
            
        public:
            
            // Init
            
            UnixClient(boost::asio::io_service &ioService,
                       const boost::asio::local::stream_protocol::endpoint &remoteEndpoint);
            
        };
        
    }
}

#include "ClientImpl.ipp"
