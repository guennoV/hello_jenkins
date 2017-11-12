//
//  Session.cpp
//  uavia-embedded
//
//  Created by Pierre Pelé on 07/09/2017.
//
//

#include "Session.hpp"

#include <iostream>

#include <boost/asio/ip/tcp.hpp>

#ifndef PROTO_HEADER_SIZE
#define PROTO_HEADER_SIZE   sizeof(uint32_t)
#endif

namespace coreKit {
    
    namespace Stream {
        
        // Session::Buffers
        
        Session::Buffers::Buffers(const ContainerType<Buffer> &buffers) : _container(buffers) { }
        Session::Buffers::Buffers(const Buffer &buffer) : Buffers(ContainerType<Buffer>( { buffer })) { }
        
        size_t Session::Buffers::getBufferSize() const {
            
            size_t result = 0;
            for (const auto &buffer : _container) {
                result += boost::asio::buffer_size(buffer);
            }
            
            return result;
        }
        
        // Session::Node
        
        Session::Node::Node(const Buffers &data,
                            const WriteCallback &writeCallback) :
        _data(data),
        _writeCallback(writeCallback)
        
        { }
        
        void Session::Node::operator()(const std::exception_ptr error) const {
            if (_writeCallback) {
                _writeCallback(error);
            }
        }
        
        // Session
        
        Session::Session() : _current(nullptr) { }
        
        void Session::read(const Callbacks &callbacks) {
            getStrand().dispatch(std::bind(&Session::read_internal, shared_from_this(),
                                           callbacks));
        }
        
        void Session::send(const Buffers &buffers,
                           const WriteCallback &writeCallback) {
            getStrand().dispatch(std::bind(&Session::send_internal, shared_from_this(),
                                           buffers,
                                           writeCallback));
        }
        
        void Session::read_internal(const Callbacks &callbacks) {
            
            // Note : Call by worker
            
            // Save callbacks
            
            _callbacks = callbacks;
            
            // Start reader
            
            boost::asio::mutable_buffer buffer(&_header._read, PROTO_HEADER_SIZE);
            read(buffer, std::bind(&Session::readHeader, shared_from_this(),
                                   std::placeholders::_1,
                                   std::placeholders::_2));
        }
        
        void Session::send_internal(const Buffers &buffers,
                                    const WriteCallback &writeCallback) {
            
            // Note : Call by worker
            
            auto node = std::make_shared<Node>(buffers, writeCallback);
            
            send(node);
        }
        
        void Session::readHeader(const boost::system::error_code &error,
                                 size_t) {
            
            // Note : Call by worker
            
            if (!error) {
                
                // Convert into host byte order
                _header._read = ntohl(_header._read);
                
                // Request the payload ...
                boost::asio::mutable_buffer buffer(_readBuffer, _header._read);
                read(buffer, std::bind(&Session::readPayload, shared_from_this(),
                                       std::placeholders::_1,
                                       std::placeholders::_2));
                
            } else {
                
                onReadError(error);
            }
        }
        
        void Session::readPayload(const boost::system::error_code &error,
                                  size_t bytesTransferred) {
            
            // Note : Call by worker
            
            if (!error) {
                
                if (_callbacks._onDataReceived) {
                    _callbacks._onDataReceived(_readBuffer, bytesTransferred);
                }
                
                // Request the header ...
                boost::asio::mutable_buffer buffer(&_header._read, PROTO_HEADER_SIZE);
                read(buffer, std::bind(&Session::readHeader, shared_from_this(),
                                       std::placeholders::_1,
                                       std::placeholders::_2));
                
            } else {
                
                onReadError(error);
            }
        }
        
        void Session::onReadError(const boost::system::error_code &error) {
            
            // Note : Call by worker
            
            // Cancel pendind operations
            
            while (!_queue.empty()) {
                
                auto node = _queue.front();
                _queue.pop();
                
                node->operator()(std::make_exception_ptr(boost::system::system_error(boost::asio::error::operation_aborted)));
            }
            
            // Inform user
            
            if (_callbacks._onReadError) {
                
                std::exception_ptr exceptionPtr = std::make_exception_ptr(boost::system::system_error(error));
                _callbacks._onReadError(exceptionPtr);
            }
            
            // Unreference callbacks
            _callbacks = Callbacks();
        }
        
        void Session::onWriteCallback(const boost::system::error_code &error, size_t) {
            
            // Note : Call by worker
            
            {
                // Call user callback
                
                std::exception_ptr exceptionPtr = nullptr;
                if (error) {
                    exceptionPtr = std::make_exception_ptr(boost::system::system_error(error));
                }
                
                _current->operator()(exceptionPtr);
                
                // Note : The buffer is now out of scope
            }
            
            // Clear the current node
            _current.reset();
            
            // Do we have any other message to send ?
            
            if (!_queue.empty()) {
                
                // Get a node from the queue
                auto node = _queue.front();
                _queue.pop();
                
                // Process the node
                process(node);
            }
            
        }
        
        void Session::process(Node::Ptr &node) {
            
            // Note : Call by worker
            
            // Erase current node
            _current = node;
            
            // Build header
            _header._write = static_cast<uint32_t>(_current->_data.getBufferSize());
            
            // Convert to network byte order
            _header._write = htonl(_header._write);
            
            // Insert header into buffers
            _current->_data._container.insert(_current->_data._container.begin(), Buffer(&_header._write, PROTO_HEADER_SIZE));
            
            // Here it is
            write(_current->_data._container,
                  std::bind(&Session::onWriteCallback, shared_from_this(),
                            std::placeholders::_1,
                            std::placeholders::_2));
        }
        
        void Session::send(Node::Ptr &node) {
            
            // Note : Call by worker
            
            if (!_current) {
                
                // Directly process the node
                
                process(node);
                
            } else {
                
                // Add the node into the queue
                
                _queue.push(node);
            }
        }
        
    }
    
}
