//
//  Server.cpp
//  embedded-software
//
//  Created by Pierre Pelé on 21/04/17.
//
//

#include "Server.hpp"

#include <future>
#include <iostream>

#include <coreKit/Log/Logger.hpp>

namespace coreKit {
    
    namespace Stream {
        
        // Our logger
        
        Logger logger( { "coreKit", "Stream" } );
        
        // Server
        
        Server::Server(boost::asio::io_service &ioService) :
        
        _strand         (ioService),
        _sessionId      (0),
        _state          (Idle)
        
        { }
        
        std::string Server::getLocalUri() {
            
            std::promise<std::string> promise;
            auto future = promise.get_future();
            
            _strand.post([this, &promise]() {
                
                if (_state != Running) {
                    promise.set_exception(std::make_exception_ptr(std::runtime_error("Server is not ready")));
                } else {
                    promise.set_value(getLocalUri_internal());
                }
            });
            
            return future.get();
        }
        
        void Server::accept(const Callbacks &callbacks,
                            const std::function<void(const std::exception_ptr)> &handler) {
            
            if (!handler) {
                throw std::invalid_argument("Handler must be defined");
            }
            
            _strand.post(std::bind(&Server::accept_internal, shared_from_this(),
                                   callbacks,
                                   handler));
        }
        
        void Server::close(const std::function<void()> &handler) {
            
            // Close task
            
            auto ptr = shared_from_this();
            auto closeTask = [ptr, handler]() {
                
                // Note : Call by worker
                
                // Check state
                
                if (ptr->_state == Idle) {
                    if (handler) {
                        handler();
                    }
                    return;
                }
                
                // Our task container
                
                struct Task {
                    
                    Task() { }
                    
                    void clean() {
                        _connections.clear();
                    }
                    
                    std::function<void()> _handler;
                    std::vector<boost::signals2::scoped_connection> _connections;
                    
                };
                
                auto taskPtr = std::make_shared<Task>();
                taskPtr->_handler = handler;
                
                // Disconnection callback
                
                auto onDisconnection = [taskPtr](const std::exception_ptr) {
                    
                    // Note : Call by worker
                    
                    if (taskPtr->_handler) {
                        taskPtr->_handler();
                    }
                    taskPtr->clean();
                };
                
                // Subscribe disconnection callbacks
                
                taskPtr->_connections.push_back(ptr->_disconnection_Signal.connect(onDisconnection));
                
                // Start closing process
                
                ptr->disconnect();
                
            };
            
            _strand.post(closeTask);
        }
        
        void Server::close(SessionId sessionIdtoClose,
                           const std::function<void()> &handler) {
            
            // Close task
            
            auto ptr = shared_from_this();
            auto closeTask = [ptr, sessionIdtoClose, handler]() {
                
                // Note : Call by worker
                
                auto iterator = ptr->_sessions._map.find(sessionIdtoClose);
                if (iterator == ptr->_sessions._map.end()) {
                    if (handler) {
                        handler();
                    }
                    
                    return;
                }
                
                // Our task container
                
                struct Task {
                    
                    Task() { }
                    
                    void clean() {
                        _connections.clear();
                    }
                    
                    std::function<void()> _handler;
                    std::vector<boost::signals2::scoped_connection> _connections;
                    
                };
                
                auto taskPtr = std::make_shared<Task>();
                taskPtr->_handler = handler;
                
                // Disconnection callback
                
                auto onDisconnection = [taskPtr, sessionIdtoClose](SessionId sessionId,
                                                                   const std::exception_ptr) {
                    
                    // Note : Call by worker
                    
                    if (sessionId == sessionIdtoClose) {
                        
                        if (taskPtr->_handler) {
                            taskPtr->_handler();
                        }
                        taskPtr->clean();
                    }
                };
                
                // Subscribe disconnection callbacks
                
                taskPtr->_connections.push_back(ptr->_sessions._disconnection_Signal.connect(onDisconnection));
                
                auto &state = iterator->second.second;
                if (state == SessionState::Connected) {
                    
                    // Close socket
                    
                    iterator->second.first->close();
                    
                    // Update session state
                    
                    state = SessionState::Disconnecting;
                }
                
            };
            
            _strand.post(closeTask);
        }
        
        void Server::send(SessionId sessionId,
                          const Session::Buffers &buffers,
                          const Session::WriteCallback &writeCallback) {
            _strand.dispatch(std::bind((void(Server::*)(SessionId,
                                                        const Session::Buffers&,
                                                        const Session::WriteCallback&)) &Server::send_internal,
                                       shared_from_this(),
                                       sessionId,
                                       buffers,
                                       writeCallback));
        }
        
        void Server::send(const Session::Buffers &buffers,
                          const Session::WriteCallback &writeCallback) {
            _strand.dispatch(std::bind((void(Server::*)(const Session::Buffers&,
                                                        const Session::WriteCallback&)) &Server::send_internal,
                                       shared_from_this(),
                                       buffers,
                                       writeCallback));
        }
        
        void Server::onNewSession(Session::Ptr sessionPtr,
                                  const boost::system::error_code &error) {
            
            // Note : Call by worker
            
            if (!error) {
                
                // Generate session ID
                const auto sessionId = getSessionId();
                
                // Inform user (This can throw)
                if (_callbacks._onNewSession) {
                    _callbacks._onNewSession(sessionId);
                }
                
                // Session callbacks
                Session::Callbacks sessionCallbacks = {
                    
                    // OnDataReceived
                    std::bind(_callbacks._onDataReceived,
                              sessionId,
                              std::placeholders::_1,
                              std::placeholders::_2),
                    
                    // OnReadError
                    _strand.wrap(std::bind(&Server::onReadError,
                                           shared_from_this(),
                                           sessionId,
                                           std::placeholders::_1))
                    
                };
                
                // Start read on socket
                sessionPtr->read(sessionCallbacks);
                
                // Save newly created session pointer
                _sessions._map.insert(std::make_pair(sessionId, std::make_pair(sessionPtr, SessionState::Connected)));
                
            } else {
                
                std::exception_ptr cause = nullptr;
                
                if (_state == Running) {
                    cause = std::make_exception_ptr(boost::system::system_error(error));
                }
                
                auto sessionCount = _sessions._map.size();
                if (sessionCount == 0) {
                    onServerClosed(cause);
                    return;
                }
                
                // Close all sessions
                
                for (const auto &session : _sessions._map) {
                    if (session.second.second != SessionState::Disconnecting) {
                        session.second.first->close();
                    }
                }
                
            }
            
        }
        
        void Server::accept_internal(const Callbacks &callbacks,
                                     const std::function<void(const std::exception_ptr)> &handler) {
            
            // Note : Call by worker
            
            // Error handling
            
            std::exception_ptr error = nullptr;
            
            // Check current state
            
            if (_state != Idle) {
                error = std::make_exception_ptr(std::runtime_error(std::string("Method [") +
                                                                   __PRETTY_FUNCTION__ +
                                                                   "] : Invalid state (" +
                                                                   std::to_string(_state) + ")"));
            }
            
            if (error) {
                handler(error);
                return;
            }
            
            // Create / Configure acceptor
            
            try  {
                configure();
            } catch (...) {
                error = std::current_exception();
            }
            
            if (!error) {
                
                _state = Running;
                
                // Save (Erase) user callbacks
                _callbacks = callbacks;
                
                // Subscribe disconnection callbacks
                
                if (callbacks._onClosed) {
                    _onDisonnection_Connection = _disconnection_Signal.connect(callbacks._onClosed, boost::signals2::connect_position::at_front);
                }
                
                if (callbacks._onSessionDisconnected) {
                    _sessions._onDisonnection_Connection = _sessions._disconnection_Signal.connect(callbacks._onSessionDisconnected, boost::signals2::connect_position::at_front);
                }
                
            } else {
                _state = Idle;
            }
            
            handler(error);
        }
        
        void Server::send_internal(SessionId sessionId,
                                   const Session::Buffers &buffers,
                                   const Session::WriteCallback &writeCallback) {
            
            // Note : Call by worker
            
            auto iterator = _sessions._map.find(sessionId);
            if (iterator != _sessions._map.end()) {
                
                if (iterator->second.second != SessionState::Connected) {
                    if (writeCallback) {
                        std::exception_ptr error = std::make_exception_ptr(std::runtime_error("Session is disconnecting"));
                        writeCallback(error);
                    }
                    
                } else {
                    
                    auto sessionsPtr = iterator->second.first;
                    sessionsPtr->Session::send(buffers, writeCallback);
                }
                
            } else {
                
                if (writeCallback) {
                    auto error = std::make_exception_ptr(std::invalid_argument("No session with ID N°" + std::to_string(sessionId)));
                    writeCallback(error);
                }
            }
        }
        
        void Server::send_internal(const Session::Buffers &buffers,
                                   const Session::WriteCallback &writeCallback) {
            
            // Note : Call by worker
            
            class Progress {
            public:
                Progress(const Session::WriteCallback &writeCallback) : _writeCallback(writeCallback) { };
                virtual ~Progress() { _writeCallback(nullptr); }
            private:
                const Session::WriteCallback _writeCallback;
            };
            
            auto progress = std::make_shared<Progress>(writeCallback);
            SessionIterator iterator;
            for (iterator = _sessions._map.begin(); iterator != _sessions._map.end(); iterator++) {
                
                if (iterator->second.second != SessionState::Connected) {
                    continue;
                }
                
                auto sessionsPtr = iterator->second.first;
                sessionsPtr->Session::send(buffers, [progress](const std::exception_ptr) { });
            }
        }
        
        void Server::onServerClosed(const std::exception_ptr cause) {
            
            // Call user callbacks
            _disconnection_Signal(cause);
            
            // Clear connections asociated to user callbacks
            _onDisonnection_Connection.release();
            _sessions._onDisonnection_Connection.release();
            
            // Unreference user callbacks
            _callbacks = Callbacks();
            
            // Update state
            _state = Idle;
            
        }
        
        void Server::onReadError(SessionId sessionId,
                                 const std::exception_ptr) {
            
            // Note : Call by worker
            
            auto iterator = _sessions._map.find(sessionId);
            
            std::exception_ptr cause = nullptr;
            if (iterator->second.second != SessionState::Disconnecting) {
                cause = std::make_exception_ptr(std::runtime_error("Error while reading on socket"));
            }
            
            // Unreference session
            _sessions._map.erase(iterator);
            
            // Call user callbacks
            _sessions._disconnection_Signal(sessionId, cause);
            
            if (_state == Closing) {
                auto sessionCount = _sessions._map.size();
                if (sessionCount == 0) {
                    onServerClosed(nullptr);
                }
            }
            
        }
        
        Server::SessionId Server::getSessionId() {
            
            unsigned int result = _sessionId.fetch_add(1);
            if (result != 0) {
                return result;
            } else {
                return getSessionId();
            }
        }
        
        void Server::disconnect(const std::exception_ptr) {
            
            // Note : Call by worker
            
            // Check current state
            if (_state == Idle ||
                _state == Closing) {
                return;
            }
            
            // Update state
            _state = Closing;
            
            // Cancel acceptor
            cancel();
        }
        
        // ScopedServer
        
        ScopedServer::ScopedServer(const std::string &localUri) {
            
            // Create server
            
            _serverPtr = makeServer(_context.getIoService(), localUri);
            
            // Start worker thread
            
            _context.start();
        }
        
        ScopedServer::~ScopedServer() {
            
            // Close server
            
            close(nullptr);
            
            // Stop worker thread
            
            _context.stop();
        }
        
        std::string ScopedServer::getLocalUri() {
            return _serverPtr->getLocalUri();
        }
        
        void ScopedServer::accept(const Callbacks &callbacks,
                                  const std::function<void(const std::exception_ptr)> &handler) {
            _serverPtr->accept(callbacks, handler);
        }
        
        void ScopedServer::close(const std::function<void()> &handler) {
            _serverPtr->close(handler);
        }
        
        void ScopedServer::close(SessionId sessionId,
                                 const std::function<void()> &handler) {
            _serverPtr->close(sessionId, handler);
        }
        
        void ScopedServer::send(SessionId sessionId,
                                const Session::Buffers &buffers,
                                const Session::WriteCallback &writeCallback) {
            _serverPtr->send(sessionId, buffers, writeCallback);
        }
        
        void ScopedServer::send(const Session::Buffers &buffers,
                                const Session::WriteCallback &writeCallback) {
            _serverPtr->send(buffers, writeCallback);
        }
        
        // Factory
        
        Server::Ptr makeServer(const std::string &localUri) {
            return std::make_shared<ScopedServer>(localUri);
        }
        
    }
}
