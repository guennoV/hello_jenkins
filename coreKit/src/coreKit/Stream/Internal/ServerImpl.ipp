//
//  ServerImpl.ipp
//  uavia-embedded
//
//  Created by Pierre Pelé on 14/09/2017.
//
//

#pragma once

#include "ServerImpl.hpp"

#include <future>

namespace coreKit {
    
    namespace Stream {
        
        // ServerImpl
        
        template <class stream_type>
        ServerImpl<stream_type>::ServerImpl(boost::asio::io_service &ioService,
                                            const onConfigureAcceptor &onConfigureAcceptor) :
        
        Server                  (ioService),
        
        _onConfigureAcceptor    (onConfigureAcceptor),
        _socket                 (ioService),
        _acceptor               (ioService)
        
        { }
        
        template <class stream_type>
        typename ServerImpl<stream_type>::Endpoint ServerImpl<stream_type>::getLocalEndpoint_internal() {
            
            // Note : Call by worker
            
            return _acceptor.local_endpoint();
        }
        
        template <class stream_type>
        void ServerImpl<stream_type>::cancel() {
            
            // Cancel acceptor
            _acceptor.cancel();
        }
        
        template <class stream_type>
        void ServerImpl<stream_type>::configure() {
            
            // Note : Call by worker
            
            std::exception_ptr error = nullptr;
            
            try  {
                // Configure acceptor
                _onConfigureAcceptor(_acceptor);
            } catch (const boost::system::system_error &exception) {
                error = std::make_exception_ptr(exception);
            } catch (...) {
                error = std::current_exception();
            }
            
            if (error) {
                std::rethrow_exception(error);
            }
            
            // Accept new connections
            _acceptor.async_accept(_socket, _endpoint,
                                   _strand.wrap(std::bind(&ServerImpl<stream_type>::onAcceptCallback,
                                                          std::dynamic_pointer_cast<ServerImpl<stream_type> >(shared_from_this()),
                                                          std::placeholders::_1)));
            
        }
        
        template <class stream_type>
        void ServerImpl<stream_type>::onAcceptCallback(const boost::system::error_code &error) {
            
            Session::Ptr sessionPtr = nullptr;
            
            if (!error) {
                sessionPtr = std::shared_ptr<SessionType>(new SessionType(std::move(_socket)));
            }
            
            try {
                onNewSession(sessionPtr, error);
            } catch (...) {
                
                // Refuse the connection
                
                _socket.close();
            }
            
            if (!error) {
                
                // Accept new connections
                _acceptor.async_accept(_socket, _endpoint,
                                       _strand.wrap(std::bind(&ServerImpl<stream_type>::onAcceptCallback,
                                                              std::dynamic_pointer_cast<ServerImpl<stream_type> >(shared_from_this()),
                                                              std::placeholders::_1)));
            }
            
        }
        
    }
}
