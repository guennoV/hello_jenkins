//
//  ServerImpl.hpp
//  uavia-embedded
//
//  Created by Pierre Pelé on 14/09/2017.
//
//

#pragma once

#include <coreKit/Stream/Server.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/local/stream_protocol.hpp>

#include "SessionImpl.hpp"

namespace coreKit {
    
    namespace Stream {
        
        // ServerImpl
        
        template <class stream_type>
        class ServerImpl : public Server {
            
        protected:
            
            // Protected declarations
            
            using Acceptor      = typename stream_type::acceptor;
            using Endpoint      = typename stream_type::endpoint;
            using Socket        = typename stream_type::socket;
            
            using SessionType   = SessionImpl<Socket>;
            
        public:
            
            // Public declarations
            
            using onConfigureAcceptor = std::function<void(Acceptor&)>;
            
            // Init
            
            ServerImpl(boost::asio::io_service &ioService,
                       const onConfigureAcceptor &onConfigureAcceptor);
            
        protected:
            
            // Protected methods
            
            // Get the local endpoint (Not thread safe)
            
            Endpoint getLocalEndpoint_internal();
            
        private:
            
            // Private methods
            
            // Cancel async operations
            
            void cancel() override;
            
            // Create / Configure acceptor
            
            void configure() override;
            
            // Callbacks
            
            void onAcceptCallback(const boost::system::error_code &error);
            
            // Attributes
            
            // Function to be called to configure acceptor
            
            const onConfigureAcceptor _onConfigureAcceptor;
            
            // Boost Asio
            
            Socket      _socket;
            Acceptor    _acceptor;
            Endpoint    _endpoint;
            
        };
        
        // TcpServer
        
        class TcpServer : public ServerImpl<boost::asio::ip::tcp> {
            
        public:
            
            // Init
            
            TcpServer(boost::asio::io_service &ioService,
                      const boost::asio::ip::tcp::endpoint &localEndpoint);
            
            // Get the local URI (Not thread safe)
            
            std::string getLocalUri_internal() override;
            
        };
        
        // UnixServer
        
        class UnixServer : public ServerImpl<boost::asio::local::stream_protocol> {
            
        public:
            
            // Init
            
            UnixServer(boost::asio::io_service &ioService,
                       const boost::asio::local::stream_protocol::endpoint &localEndpoint);
            
            // Get the local URI (Not thread safe)
            
            std::string getLocalUri_internal() override;
            
        };
        
    }
}

#include "ServerImpl.ipp"
