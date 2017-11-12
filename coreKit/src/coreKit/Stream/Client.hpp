//
//  Client.hpp
//  embedded-software
//
//  Created by Pierre Pelé on 21/04/17.
//
//

#pragma once

#include <stdint.h>

#include <functional>
#include <memory>
#include <string>

#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/signals2/signal.hpp>

#include <coreKit/Utils/Context.hpp>

#include "Session.hpp"

namespace coreKit {
    
    namespace Stream  {
        
        // Client_Interface
        
        class Client_Interface {
            
        public:
            
            // Declarations
            
            using OnConnectionClose = std::function<void(const std::exception_ptr)>;
            
            // Callbacks
            
            struct Callbacks {
                
                // Attributes
                
                // Function to be called on incomming data
                
                Session::OnDataReceived     _onDataReceived;
                
                // Function to be called on disconnection
                
                OnConnectionClose           _onConnectionClose;
                
            };
            
            // Interface declarations
            
            // Open
            
            virtual void open(const Callbacks &callbacks,
                              const std::function<void(const std::exception_ptr)> &handler,
                              uint64_t timeout_ms) = 0;
            
            // Close
            
            virtual void close(const std::function<void()> &handler) = 0;
            
            // Send data
            
            virtual void send(const Session::Buffers &buffers,
                              const Session::WriteCallback &writeCallback) = 0;
            
        };
        
        // Client
        
        class Client :
        public Client_Interface,
        public std::enable_shared_from_this<Client> {
            
        public:
            
            // Public declarations
            
            using Ptr = std::shared_ptr<Client_Interface>;
            
            // Init
            
            Client(boost::asio::io_service &ioService);
            
            /* Non-copyable.*/
            Client(const Client&) = delete;
            Client & operator=(const Client&) = delete;
            
            // Client interface
            
            // Open
            
            void open(const Callbacks &callbacks,
                      const std::function<void(const std::exception_ptr)> &handler,
                      uint64_t timeout_ms) override;
            
            // Close
            
            void close(const std::function<void()> &handler) override;
            
            // Send data
            
            void send(const Session::Buffers &buffers,
                      const Session::WriteCallback &writeCallback) override;
            
        protected:
            
            // Protected attributes
            
            // Time handling
            
            boost::asio::strand _strand;
            
            // The session itself
            
            Session::Ptr _sessionPtr;
            
        private:
            
            // Private declarations
            
            // State
            
            enum State {
                
                Disconnected    = 0,
                Connecting      = 1,
                Connected       = 2,
                Disconnecting   = 3
                
            };
            
            // Private methods
            
            void open_internal(const Callbacks &callbacks,
                               const std::function<void(const std::exception_ptr)> &handler,
                               uint64_t timeout_ms);
            
            void send_internal(const Session::Buffers &buffers,
                               const Session::WriteCallback &writeCallback);
            
            // Callbacks / Disconnection handling
            
            void onSocketConnected(const Session::OnDataReceived &onDataReceived);
            
            void onDisconnection(const std::exception_ptr cause);
            
            void onReadError(const std::exception_ptr cause);
            
            void disconnect(const std::exception_ptr cause);
            
            // Create / Configure session
            
            virtual void configure(const std::function<void(const boost::system::error_code&)> &handler) = 0;
            
            // Private attributes
            
            // Current state
            
            State _state;
            
            // Disconnection handling
            
            boost::signals2::signal<void(const std::exception_ptr)>     _disconnection_Signal;
            boost::signals2::scoped_connection                          _onDisonnection_Connection;
            
        };
        
        // Factory
        
        Client::Ptr makeClient(boost::asio::io_service &ioService,
                               const std::string &remoteUri);
        
        // ScopedClient
        
        class ScopedClient : public Client_Interface {
            
        public:
            
            // Init
            
            ScopedClient(const std::string &remoteUri);
            virtual ~ScopedClient();
            
            /* Non-copyable.*/
            ScopedClient(const ScopedClient&) = delete;
            ScopedClient & operator=(const ScopedClient&) = delete;
            
            // Client interface
            
            // Open
            
            void open(const Callbacks &callbacks,
                      const std::function<void(const std::exception_ptr)> &handler,
                      uint64_t timeout_ms) override;
            
            // Close
            
            void close(const std::function<void()> &handler) override;
            
            // Send data
            
            void send(const Session::Buffers &buffers,
                      const Session::WriteCallback &writeCallback) override;
            
        private:
            
            // Attributes
            
            // Our context
            
            Context         _context;
            
            // The client itself
            
            Client::Ptr     _clientPtr;
            
        };
        
        // Factory
        
        Client::Ptr makeClient(const std::string &remoteUri);
        
    }
}
