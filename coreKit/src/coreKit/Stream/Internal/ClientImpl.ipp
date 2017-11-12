//
//  ClientImpl.ipp
//  uavia-embedded
//
//  Created by Pierre Pelé on 14/09/2017.
//
//

#pragma once

#include "ClientImpl.hpp"

#include <future>

namespace coreKit {
    
    namespace Stream  {
        
        // ClientImpl
        
        template <class stream_type>
        ClientImpl<stream_type>::ClientImpl(boost::asio::io_service &ioService,
                                            const onConfigureSocket &onConfigureSocket) :
        
        Client              (ioService),
        _onConfigureSocket  (onConfigureSocket)
        
        {
            if (!onConfigureSocket) {
                throw std::invalid_argument("'onConfigureSocket' callback must be defined");
            }
        }
        
        template <class stream_type>
        void ClientImpl<stream_type>::configure(const std::function<void(const boost::system::error_code&)> &handler) {
            
            // Note : Call by worker
            
            std::exception_ptr error = nullptr;
            
            // Create (Erase) session
            
            auto sessionPtr = std::shared_ptr<SessionType>(new SessionType(Socket(_strand.get_io_service())));
            auto &socket    = sessionPtr->socket();
            
            Endpoint endpoint;
            
            try  {
                // Configure socket
                endpoint = _onConfigureSocket(socket);
            } catch (const boost::system::system_error &exception) {
                error = std::make_exception_ptr(exception);
            } catch (...) {
                error = std::current_exception();
            }
            
            if (error) {
                std::rethrow_exception(error);
            } else {
                _sessionPtr = sessionPtr;
            }
            
            // Connect
            socket.async_connect(endpoint, _strand.wrap(handler));
        }
        
    }
}
