//
//  Session.hpp
//  uavia-embedded
//
//  Created by Pierre Pelé on 07/09/2017.
//
//

#pragma once

#include <stdint.h>

#include <functional>
#include <memory>
#include <queue>
#include <vector>

#include <boost/asio/buffer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>

namespace coreKit {
    
    namespace Stream {
        
        // Session
        
        class Session :
        public std::enable_shared_from_this<Session> {
            
        public:
            
            // Public declarations
            
            // Declarations
            
            using Ptr               = std::shared_ptr<Session>;
            using Buffer            = boost::asio::const_buffer;
            using WriteCallback     = std::function<void(const std::exception_ptr)>;
            using OnReadError       = std::function<void(const std::exception_ptr)>;
            using OnDataReceived    = std::function<void(const void*,
                                                         size_t)>;
            
            template <class T>
            using ContainerType     = std::vector<T>;
            
            // Callbacks
            
            struct Callbacks {
                
                // Attributes
                
                // Function to be called on incomming data
                
                OnDataReceived  _onDataReceived;
                
                // Function to be called on read errors
                
                OnReadError     _onReadError;
                
            };
            
            // Buffers
            
            struct Buffers {
                
                Buffers(const ContainerType<Buffer> &buffers);
                Buffers(const Buffer &buffer);
                size_t getBufferSize() const;
                ContainerType<Buffer> _container;
                
            };
            
            // Init
            
            Session();
            
            /* Non-copyable.*/
            Session(const Session&) = delete;
            Session & operator=(const Session&) = delete;
            
            // Public interface
            
            // Start reading on socket
            
            void read(const Callbacks &callbacks);
            
            // Send data
            
            void send(const Buffers &buffers,
                      const WriteCallback &writeCallback);
            
            // Close session
            
            virtual void close() = 0;
            
        protected:
            
            // Protected methods
            
            // Time handling
            
            virtual boost::asio::strand& getStrand() = 0;
            
        private:
            
            // Private declarations
            
            // Node
            
            struct Node {
                
                using Ptr = std::shared_ptr<Node>;
                
                Node(const Buffers &data,
                     const WriteCallback &writeCallback);
                
                /* Non-copyable.*/
                Node(const Node&) = delete;
                Node & operator=(const Node&) = delete;
                
                void operator()(const std::exception_ptr error) const;
                
                Buffers         _data;
                WriteCallback   _writeCallback;
                
            };
            
            // Private methods
            
            void read_internal(const Callbacks &callbacks);
            
            void send_internal(const Buffers &buffers,
                               const WriteCallback &writeCallback);
            
            // Read operationd
            
            virtual void read(boost::asio::mutable_buffer &buffer,
                              const std::function<void(const boost::system::error_code&, size_t)> &handler) = 0;
            
            void readHeader(const boost::system::error_code &error,
                            size_t bytesTransferred);
            
            void readPayload(const boost::system::error_code &error,
                             size_t bytesTransferred);
            
            void onReadError(const boost::system::error_code &error);
            
            // Write operations
            
            virtual void write(const Buffers &buffers,
                               const std::function<void(const boost::system::error_code&, size_t)> &handler) = 0;
            
            void onWriteCallback(const boost::system::error_code &error,
                                 size_t bytesTransferred);
            
            void process(Node::Ptr &node);
            
            void send(Node::Ptr &node);
            
            // Attributes
            
            // User callbacks
            
            Callbacks               _callbacks;
            
            // Our read buffer
            
            uint8_t                 _readBuffer[UINT16_MAX];
            
            // Header buffer
            
            struct {
                
                uint32_t            _write;
                uint32_t            _read;
                
            } _header;
            
            // Write queue
            
            std::queue<Node::Ptr>   _queue;
            
            // Write packet being processed if any
            
            Node::Ptr               _current;
            
        };
        
    }
    
}
