//
//  SessionImpl.hpp
//  uavia-embedded
//
//  Created by Pierre Pelé on 14/09/2017.
//
//

#pragma once

#include <memory>

#include <coreKit/Stream/Session.hpp>

namespace coreKit {
    
    namespace Stream {
        
        // SessionImpl
        
        template <class Socket>
        class SessionImpl : public Session {
            
        public:
            
            // Init
            
            SessionImpl(Socket socket);
            
            // Access to the socket
            
            Socket& socket();
            
        private:
            
            // Private methods
            
            // Time handling
            
            boost::asio::strand& getStrand() override;
            
            // Read operationd
            
            void read(boost::asio::mutable_buffer &buffer,
                      const std::function<void(const boost::system::error_code&,
                                               size_t)> &handler) override;
            
            // Write operations
            
            void write(const Buffers &buffers,
                       const std::function<void(const boost::system::error_code&,
                                                size_t)> &handler) override;
            
            // Close session
            
            void close() override;
            
            // Attributes
            
            // Time handling
            
            boost::asio::strand _strand;
            
            // The socket itself
            
            Socket _socket;
            
        };
        
    }
}

#include "SessionImpl.ipp"
