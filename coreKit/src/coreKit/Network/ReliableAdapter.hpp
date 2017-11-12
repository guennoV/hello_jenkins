//
//  ReliableAdapter.hpp
//  coreKit
//
//  Created by Pierre Pelé on 23/10/2017.
//

#pragma once

#include <stdint.h>

#include <memory>

#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/signals2/signal.hpp>

#include "Adapter.hpp"

namespace coreKit { namespace Network {
    
    // Declarations
    
    namespace iTC {
        
        // Generic declarations
        
        using MessageId = uint32_t;
        
        // Protocol definition
        
        // MessageType
        
        enum class PayloadType {
            
            Message     = 0,
            Ack         = 1
            
        };
        
        // AckCode
        
        enum class AckCode {
            
            Ok          = 0
            
        };
        
        // Header
        
        struct Header {
            
            PayloadType _payloadType;
            MessageId   _messageId;
            
        };
        
        // Packet
        
        template <class T> struct Packet {
            
            using PayloadType = T;
            
            Packet(const Header &header,
                   const T &payload) :
            
            _header(header), _payload(payload)
            
            { }
            
            // Attributes
            
            const Header _header;
            const T _payload;
            
        };
        
        namespace Payload {
            
            // Ack
            
            struct Ack {
                
                uint8_t     _count;
                AckCode     _code;
                
            };
            
            // Message
            
            struct Message {
                
                uint8_t     _count;
                Buffer      _body;
                
            };
            
        }
        
        using Ack = Packet<Payload::Ack>;
        using Message = Packet<Payload::Message>;
        
    }
    
    // Opaque declarations
    
    struct ReliableTask;
    
    // ReliableAdapter
    
    class ReliableAdapter :
    public Adapter,
    public std::enable_shared_from_this<ReliableAdapter> {
        
        friend struct ReliableTask;
        
    public:
        
        // Declarations
        
        using Ptr = std::shared_ptr<ReliableAdapter>;
        
        // Config
        
        struct Config {
            
            // Attributes
            
            // Note : Timeout are in microseconds
            
            // Global message timeout
            
            uint64_t _globalTimeout;
            
            // Message timeout generator
            
            std::function<uint64_t(size_t)> _timeoutFunc;
            
        };
        
        // Init
        
        ReliableAdapter(boost::asio::io_service &ioService,
                        const Config &config);
        
        // Initialize / Cancel
        
        void init(const Adapter::Callbacks &callbacks) override;
        void cancel() override;
        
        // Adapter interface
        
        void handleIncommingData(const Buffer &buffer,
                                 const WriteCallback &writeCallback) override;
        
        void handleOutgoingData(const Buffer &buffer,
                                const WriteCallback &writeCallback) override;
        
    private:
        
        // Private declations
        
        // State
        
        enum State {
            
            Started         = 0,
            Stopped         = 1
            
        };
        
        // HandledMessage
        
        struct HandledMessage {
            
            // Declarations
            
            using Ptr = std::shared_ptr<HandledMessage>;
            
            // Methods
            
            HandledMessage(const iTC::Message &message,
                           iTC::AckCode code);
            
            iTC::Ack makeAck();
            
            // Attributes
            
            // The message Id
            
            iTC::MessageId   _messageId;
            
            // The ack code for this message
            
            iTC::AckCode    _ackCode;
            
            // Number of time ack has been sent
            // for this message
            // Must be > 1
            
            uint8_t         _count;
            
        };
        
        // AckResult
        
        struct AckResult {
            
            typedef bool result_type;
            
            template<typename InputIterator>
            bool operator()(InputIterator first,
                            InputIterator last) const {
                
                while (first != last) {
                    if (*first) {
                        return true;
                    }
                    ++first;
                }
                
                return false;
            }
            
        };
        
        // Private methods
        
        void handleIncommingData_internal(const Buffer &buffer,
                                          const WriteCallback &writeCallback);
        
        void handleOutgoingData_internal(const Buffer &buffer,
                                         const WriteCallback &writeCallback);
        
        // Helpers
        
        void send(std::shared_ptr<uint8_t> buffer,
                  size_t bufferSize,
                  const WriteCallback &writeCallback);
        
        void send(const iTC::Message &message,
                  const WriteCallback &writeCallback);
        
        void send(const iTC::Ack &ack,
                  const WriteCallback &writeCallback);
        
        static uint64_t defaultTimeoutFunc(size_t count);
        
        uint64_t timeoutFunc(size_t count);
        
        // Functions to be call to handle incomming data
        
        void onIncommingMessage(const iTC::Message &message,
                                const WriteCallback &writeCallback);
        
        bool onIncommingAck(const iTC::Ack &ack);
        
        // Cancel a specific task
        
        bool cancel(iTC::MessageId messageId);
        
        // Message Ids generator
        
        iTC::MessageId getMessageId();
        
        // Disconnection handling
        
        void disconnect();
        
        // Attributes
        
        // Time handling
        
        boost::asio::strand _strand;
        
        // Our config
        
        const Config _config;
        
        // Active state
        
        State _state;
        
        // User callbacks
        
        Adapter::Callbacks _callbacks;
        
        // Active message Id
        
        std::atomic_uint _messageId;
        
        // Here are all handled messages
        
        std::map<iTC::MessageId, HandledMessage::Ptr> _handledMessages;
        
        // Acknowledgment handling
        
        boost::signals2::signal<bool(const iTC::Ack&), AckResult> _acks_Signal;
        
        // Task abortion handling
        
        boost::signals2::signal<bool(iTC::MessageId), AckResult> _cancel_Signal;
        
        // Disconnection handling
        
        boost::signals2::signal<void()>     _disconnection_Signal;
        boost::signals2::scoped_connection  _onDisonnection_Connection;
        
    };
    
} }
