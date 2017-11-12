//
//  SessionImpl.ipp
//  uavia-embedded
//
//  Created by Pierre Pelé on 14/09/2017.
//
//

#pragma once

#include "SessionImpl.hpp"

#include <boost/asio/write.hpp>
#include <boost/asio/read.hpp>

namespace coreKit {
    
    namespace Stream {
        
        // Session
        
        template <class Socket>
        SessionImpl<Socket>::SessionImpl(Socket socket) :
        _strand(socket.get_io_service()),
        _socket(std::move(socket))
        
        { }
        
        template <class Socket>
        Socket& SessionImpl<Socket>::socket() {
            return _socket;
        }
        
        template <class Socket>
        boost::asio::strand& SessionImpl<Socket>::getStrand() {
            return _strand;
        }
        
        template <class Socket>
        void SessionImpl<Socket>::read(boost::asio::mutable_buffer &buffer,
                                       const std::function<void(const boost::system::error_code&, size_t)> &handler) {
            boost::asio::async_read(_socket, boost::asio::buffer(buffer), _strand.wrap(handler));
        }
        
        template <class Socket>
        void SessionImpl<Socket>::write(const Buffers &buffers,
                                        const std::function<void(const boost::system::error_code&, size_t)> &handler) {
            boost::asio::async_write(_socket, buffers._container, _strand.wrap(handler));
        }
        
        template <class Socket>
        void SessionImpl<Socket>::close() {
            
            // Close socket & Cancel pending async operations
            
            _socket.close();
        }
        
    }
}
