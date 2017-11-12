//
//  Client.cpp
//  embedded-software
//
//  Created by Pierre Pelé on 21/04/17.
//
//

#include "Client.hpp"

#include <boost/asio/deadline_timer.hpp>

#include <iostream>
#include <future>

namespace coreKit {
    
    namespace Stream  {
        
        // Client
        
        Client::Client(boost::asio::io_service &ioService) :
        
        _strand         (ioService),
        _sessionPtr     (nullptr),
        _state          (Disconnected)
        
        { }
        
        void Client::open(const Callbacks &callbacks,
                          const std::function<void(const std::exception_ptr)> &handler,
                          uint64_t timeout_ms) {
            
            if (!handler) {
                throw std::invalid_argument("Handler must be defined");
            }
            
            _strand.post(std::bind(&Client::open_internal, shared_from_this(),
                                   callbacks,
                                   handler,
                                   timeout_ms));
        }
        
        void Client::close(const std::function<void()> &handler) {
            
            // Close task
            
            auto ptr = shared_from_this();
            auto closeTask = [ptr, handler]() {
                
                // Note : Call by worker
                
                // Check state
                
                if (ptr->_state == Disconnected) {
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
                
                // Close socket
                
                ptr->disconnect(nullptr);
                
            };
            
            _strand.post(closeTask);
        }
        
        void Client::send(const Session::Buffers &buffers,
                          const Session::WriteCallback &writeCallback) {
            _strand.dispatch(std::bind(&Client::send_internal, shared_from_this(),
                                       buffers,
                                       writeCallback));
        }
        
        void Client::open_internal(const Callbacks &callbacks,
                                   const std::function<void(const std::exception_ptr)> &handler,
                                   uint64_t timeout_ms) {
            
            // Note : Call by worker
            
            // Our task container
            
            struct Task {
                
                Task(boost::asio::io_service &ioService) : _finished(false), _timer(ioService) { }
                
                void clean() {
                    _timer.cancel();
                    _connections.clear();
                }
                
                bool _finished;
                boost::asio::deadline_timer _timer;
                std::function<void(const std::exception_ptr)> _handler;
                std::vector<boost::signals2::scoped_connection> _connections;
                
            };
            
            auto taskPtr = std::make_shared<Task>(_strand.get_io_service());
            taskPtr->_handler = handler;
            
            // Disconnection callback
            
            auto onDisconnection = [taskPtr](const std::exception_ptr cause) {
                
                // Note : Call by worker
                
                if (!taskPtr->_finished) {
                    
                    taskPtr->_finished = true;
                    taskPtr->_handler(cause);
                    taskPtr->clean();
                }
            };
            
            // Timer callback
            
            auto ptr = shared_from_this();
            auto timerCallback = [ptr, taskPtr](const boost::system::error_code &error) {
                
                // Note : Call by worker
                
                if (!error && !taskPtr->_finished) {
                    
                    auto cause = std::make_exception_ptr(std::runtime_error("Connection timed out"));
                    ptr->disconnect(cause);
                }
            };
            
            // Error handling
            
            std::exception_ptr error = nullptr;
            
            // Check current state
            
            if (_state != Disconnected) {
                error = std::make_exception_ptr(std::runtime_error(std::string("Method [") +
                                                                   __PRETTY_FUNCTION__ +
                                                                   "] : Invalid state (" +
                                                                   std::to_string(_state) + ")"));
            }
            
            if (error) {
                taskPtr->_handler(error);
                return;
            }
            
            // Create / Configure session
            
            try  {
                
                configure([ptr, taskPtr, callbacks](const boost::system::error_code &errorCode) {
                    
                    // Note : Call by worker
                    
                    if (!taskPtr->_finished) {
                        
                        std::exception_ptr error = nullptr;
                        if (errorCode) {
                            error = std::make_exception_ptr(boost::system::system_error(errorCode));
                        }
                        
                        if (error) {
                            ptr->disconnect(error);
                        } else {
                            
                            if (callbacks._onConnectionClose) {
                                ptr->_onDisonnection_Connection = ptr->_disconnection_Signal.connect(callbacks._onConnectionClose, boost::signals2::connect_position::at_front);
                            }
                            
                            ptr->onSocketConnected(callbacks._onDataReceived);
                            
                            taskPtr->_finished = true;
                            taskPtr->_handler(nullptr);
                            taskPtr->clean();
                        }
                    }
                    
                });
                
            } catch (...) {
                error = std::current_exception();
            }
            
            if (error) {
                taskPtr->_handler(error);
                return;
            }
            
            // Subscribe disconnection callbacks
            taskPtr->_connections.push_back(_disconnection_Signal.connect(onDisconnection));
            
            // Start timer
            taskPtr->_timer.expires_from_now(boost::posix_time::milliseconds(timeout_ms));
            taskPtr->_timer.async_wait(_strand.wrap(timerCallback));
            
            // Update state
            _state = Connecting;
        }
        
        void Client::send_internal(const Session::Buffers &buffers,
                                   const Session::WriteCallback &writeCallback) {
            
            // Note : Call by worker
            
            if (_state != Connected) {
                if (writeCallback) {
                    std::exception_ptr error = std::make_exception_ptr(std::runtime_error("Session is not ready"));
                    writeCallback(error);
                }
                
                return;
            }
            
            _sessionPtr->Session::send(buffers, writeCallback);
        }
        
        void Client::onSocketConnected(const Session::OnDataReceived &onDataReceived) {
            
            // Note : Call by worker
            
            // Update current state
            _state = Connected;
            
            // Session callbacks
            Session::Callbacks sessionCallbacks = {
                onDataReceived,
                _strand.wrap(std::bind(&Client::onReadError, shared_from_this(), std::placeholders::_1))
            };
            
            // Start reader
            _sessionPtr->Session::read(sessionCallbacks);
        }
        
        void Client::onDisconnection(const std::exception_ptr cause) {
            
            // Note : Call by worker
            
            // Check current state
            
            if (_state == Disconnected) {
                return;
            }
            
            // Update the state
            _state = Disconnected;
            
            // Call user callbacks
            _disconnection_Signal(cause);
            
            // Clear connection asociated to user callbacks
            _onDisonnection_Connection.release();
            
            // Unreference session
            _sessionPtr = nullptr;
        }
        
        void Client::onReadError(const std::exception_ptr) {
            
            // Note : Call by worker
            
            std::exception_ptr cause = nullptr;
            if (_state != Disconnecting) {
                cause = std::make_exception_ptr(std::runtime_error("Error while reading on socket"));
            }
            
            onDisconnection(cause);
            
        }
        
        void Client::disconnect(const std::exception_ptr cause) {
            
            // Note : Call by worker
            
            // Check current state
            
            if (_state == Disconnected ||
                _state == Disconnecting) {
                return;
            }
            
            if (_state == Connecting) {
                
                // Synchonous disconnection
                
                onDisconnection(cause);
                
            } else {
                
                // Asynchonous disconnection
                
                // Update state
                _state = Disconnecting;
                
                // Cancel async operations
                _sessionPtr->close();
            }
            
        }
        
        // ScopedClient
        
        ScopedClient::ScopedClient(const std::string &remoteUri) {
            
            // Create client
            _clientPtr = makeClient(_context.getIoService(), remoteUri);
            
            // Start worker thread(s)
            _context.start();
        }
        
        ScopedClient::~ScopedClient() {
            
            // Close client
            close(nullptr);
            
            // Stop worker thread(s)
            _context.stop();
        }
        
        void ScopedClient::open(const Callbacks &callbacks,
                                const std::function<void(const std::exception_ptr)> &handler,
                                uint64_t timeout_ms) {
            _clientPtr->open(callbacks, handler, timeout_ms);
        }
        
        void ScopedClient::close(const std::function<void()> &handler) {
            _clientPtr->close(handler);
        }
        
        void ScopedClient::send(const Session::Buffers &buffers,
                                const Session::WriteCallback &writeCallback) {
            _clientPtr->send(buffers, writeCallback);
        }
        
        // Factory
        
        Client::Ptr makeClient(const std::string &remoteUri) {
            return std::make_shared<ScopedClient>(remoteUri);
        }
        
    }
}
