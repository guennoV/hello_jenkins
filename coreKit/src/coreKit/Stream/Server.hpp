//
//  Server.hpp
//  embedded-software
//
//  Created by Pierre Pelé on 21/04/17.
//
//

#pragma once

#include <stdint.h>

#include <atomic>
#include <functional>
#include <map>
#include <memory>

#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/signals2/signal.hpp>

#include <coreKit/Utils/Context.hpp>

#include "Session.hpp"

namespace coreKit {
    
    namespace Stream {
        
        // Sever_Interface
        
        class Server_Interface {
            
        public:
            
            // Declarations
            
            using SessionId = size_t;
            using OnDataReceived = std::function<void(SessionId sessionId,
                                                      const void* data,
                                                      size_t datalength)>;
            using OnClosed = std::function<void(const std::exception_ptr)>;
            using OnNewSession = std::function<void(SessionId sessionId)>;
            using OnSessionDisconnected = std::function<void(SessionId sessionId,
                                                             const std::exception_ptr)>;
            
            // Callbacks
            
            struct Callbacks {
                
                // Attributes
                
                // Function to be called on incoming data
                
                OnDataReceived          _onDataReceived;
                
                // Function to be called on server disconnection
                
                OnClosed                _onClosed;
                
                // Function to be called on new session
                
                OnNewSession            _onNewSession;
                
                // Function to be called on session disconnection
                
                OnSessionDisconnected   _onSessionDisconnected;
                
            };
            
            // Server Interface
            
            // Get the local URI
            
            virtual std::string getLocalUri() = 0;
            
            // Accept
            
            virtual void accept(const Callbacks &callbacks,
                                const std::function<void(const std::exception_ptr)> &handler) = 0;
            
            // Close / Disocnnect server (Will disconnect all sessions)
            
            virtual void close(const std::function<void()> &handler) = 0;
            
            // Close / Disconnect a specific session
            
            virtual void close(SessionId sessionId,
                               const std::function<void()> &handler) = 0;
            
            // Send data to a specific session
            
            virtual void send(SessionId sessionId,
                              const Session::Buffers &buffers,
                              const Session::WriteCallback &writeCallback) = 0;
            
            // Send data to all session(s) (multicast)
            
            virtual void send(const Session::Buffers &buffers,
                              const Session::WriteCallback &writeCallback) = 0;
            
        };
        
        // Server
        
        class Server :
        public Server_Interface,
        public std::enable_shared_from_this<Server> {
            
        public:
            
            // Public declarations
            
            using Ptr = std::shared_ptr<Server_Interface>;
            
            // Init
            
            Server(boost::asio::io_service &ioService);
            
            /* Non-copyable.*/
            Server(const Server&) = delete;
            Server & operator=(const Server&) = delete;
            
            // Server Interface
            
            // Get the local URI
            
            std::string getLocalUri() override;
            
            // Accept
            
            void accept(const Callbacks &callbacks,
                        const std::function<void(const std::exception_ptr)> &handler) override;
            
            // Close / Disocnnect server (Will disconnect all sessions)
            
            void close(const std::function<void()> &handler) override;
            
            // Close / Disconnect a specific session
            
            void close(SessionId sessionId,
                       const std::function<void()> &handler) override;
            
            // Send data to a specific session
            
            void send(SessionId sessionId,
                      const Session::Buffers &buffers,
                      const Session::WriteCallback &writeCallback) override;
            
            // Send data to all session(s) (broadcast)
            
            void send(const Session::Buffers &buffers,
                      const Session::WriteCallback &writeCallback) override;
            
        protected:
            
            // Protected attributes
            
            // Time handling
            
            boost::asio::strand _strand;
            
            // Protected methods
            
            // Callbacks
            
            void onNewSession(Session::Ptr sessionPtr,
                              const boost::system::error_code &error);
        private:
            
            // Private declarations
            
            // State
            
            enum State {
                
                Idle            = 0,
                Running         = 1,
                Closing         = 2
                
            };
            
            // SessionState
            
            enum class SessionState {
                
                Connected       = 0,
                Disconnecting   = 1
                
            };
            
            // Other declarations
            
            using SessionMap = std::map<SessionId, std::pair<Session::Ptr, SessionState> >;
            using SessionIterator = SessionMap::iterator;
            
            // Private methods
            
            void accept_internal(const Callbacks &callbacks,
                                 const std::function<void(const std::exception_ptr)> &handler);
            
            void send_internal(SessionId sessionId,
                               const Session::Buffers &buffers,
                               const Session::WriteCallback &writeCallback);
            
            void send_internal(const Session::Buffers &buffers,
                               const Session::WriteCallback &writeCallback);
            
            // Callbacks / Disconnection handling
            
            void onServerClosed(const std::exception_ptr cause);
            
            void onReadError(SessionId sessionId,
                             const std::exception_ptr error);
            
            // Session Ids generator
            
            SessionId getSessionId();
            
            // Disconnection handling
            
            // Disconnect all sessions and close server
            
            void disconnect(const std::exception_ptr cause = nullptr);
            
            // Get the local URI (Not thread safe)
            
            virtual std::string getLocalUri_internal() = 0;
            
            // Cancel async operations
            
            virtual void cancel() = 0;
            
            // Create / Configure acceptor
            
            virtual void configure() = 0;
            
            // Private attributes
            
            // Active session Id
            
            std::atomic_uint _sessionId;
            
            // Current state
            
            State _state;
            
            // User callbacks
            
            Callbacks _callbacks;
            
            // Disconnection handling
            
            boost::signals2::signal<void(const std::exception_ptr)>     _disconnection_Signal;
            boost::signals2::scoped_connection                          _onDisonnection_Connection;
            
            // Session handling
            
            struct {
                
                SessionMap _map;
                
                boost::signals2::signal<void(SessionId,
                                             const std::exception_ptr)> _disconnection_Signal;
                boost::signals2::scoped_connection                      _onDisonnection_Connection;
                
            } _sessions;
            
        };
        
        // Factory
        
        Server::Ptr makeServer(boost::asio::io_service &ioService,
                               const std::string &localUri);
        
        // ScopedServer
        
        class ScopedServer : public Server_Interface {
            
        public:
            
            // Init
            
            ScopedServer(const std::string &localUri);
            virtual ~ScopedServer();
            
            /* Non-copyable.*/
            ScopedServer(const ScopedServer&) = delete;
            ScopedServer & operator=(const ScopedServer&) = delete;
            
            // Server Interface
            
            // Get the local URI
            
            std::string getLocalUri() override;
            
            // Accept
            
            void accept(const Callbacks &callbacks,
                        const std::function<void(const std::exception_ptr)> &handler) override;
            
            // Close / Disocnnect server (Will disconnect all sessions)
            
            void close(const std::function<void()> &handler) override;
            
            // Close / Disconnect a specific session
            
            void close(SessionId sessionId,
                       const std::function<void()> &handler) override;
            
            // Send data to a specific session
            
            void send(SessionId sessionId,
                      const Session::Buffers &buffers,
                      const Session::WriteCallback &writeCallback) override;
            
            // Send data to all session(s) (broadcast)
            
            void send(const Session::Buffers &buffers,
                      const Session::WriteCallback &writeCallback) override;
            
        private:
            
            // Attributes
            
            // Our context
            
            Context         _context;
            
            // The client itself
            
            Server::Ptr     _serverPtr;
            
        };
        
        // Factory
        
        Server::Ptr makeServer(const std::string &localUri);
        
    }
}
