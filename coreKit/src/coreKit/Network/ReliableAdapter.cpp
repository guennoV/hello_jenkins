//
//  ReliableAdapter.cpp
//  coreKit
//
//  Created by Pierre Pelé on 23/10/2017.
//

#include "ReliableAdapter.hpp"

#include <boost/asio/deadline_timer.hpp>

// iTC Protocol defines

// Header

#define ITC_HEADER_PAYLOAD_TYPE_OFFSET      0
#define ITC_HEADER_PAYLOAD_TYPE_SIZE        sizeof(uint8_t)

#define ITC_HEADER_MESSAGE_ID_OFFSET        ITC_HEADER_PAYLOAD_TYPE_SIZE
#define ITC_HEADER_MESSAGE_ID_SIZE          sizeof(uint32_t)

#define ITC_HEADER_SIZE                     (ITC_HEADER_PAYLOAD_TYPE_SIZE + ITC_HEADER_MESSAGE_ID_SIZE)

// Payloads

#define ITC_GET_PAYLOAD(data)               &((uint8_t*) data)[ITC_HEADER_SIZE]

// Ack

#define ITC_PAYLOAD_ACK_COUNT_OFFSET        0
#define ITC_PAYLOAD_ACK_COUNT_SIZE          sizeof(uint8_t)

#define ITC_PAYLOAD_ACK_CODE_OFFSET         ITC_PAYLOAD_ACK_COUNT_SIZE
#define ITC_PAYLOAD_ACK_CODE_SIZE           sizeof(uint8_t)

#define ITC_PAYLOAD_ACK_SIZE                (ITC_PAYLOAD_ACK_COUNT_SIZE + ITC_PAYLOAD_ACK_CODE_SIZE)

// Message

#define ITC_PAYLOAD_MESSAGE_COUNT_OFFSET    0
#define ITC_PAYLOAD_MESSAGE_COUNT_SIZE      sizeof(uint8_t)

#define ITC_PAYLOAD_MESSAGE_BODY_OFFSET     ITC_PAYLOAD_MESSAGE_COUNT_SIZE
#define ITC_PAYLOAD_MESSAGE_MINIMAL_SIZE    ITC_PAYLOAD_MESSAGE_COUNT_SIZE

// Packets (Header + Payload)

#define ITC_ACK_SIZE                        (ITC_HEADER_SIZE + ITC_PAYLOAD_ACK_SIZE)
#define ITC_MESSAGE_MINIMAL_SIZE            (ITC_HEADER_SIZE + ITC_PAYLOAD_MESSAGE_MINIMAL_SIZE)

namespace coreKit { namespace Network {
    
    namespace iTC {
        
        void decode(const void* data,
                    size_t bufferSize,
                    iTC::Payload::Message &message) {
            
            // Check paquet size
            if (bufferSize < ITC_MESSAGE_MINIMAL_SIZE) {
                throw std::runtime_error("Packet has invalid size");
            }
            
            // Read count
            message._count = *((uint8_t*) data);
            
            // Read body
            size_t payloadSize = (bufferSize - ITC_HEADER_SIZE);
            if (payloadSize == 0) {
                throw std::runtime_error("Body has invalid size");
            }
            
            message._body = boost::asio::const_buffer(&(((uint8_t*) data)[ITC_PAYLOAD_MESSAGE_BODY_OFFSET]), (payloadSize - ITC_PAYLOAD_MESSAGE_MINIMAL_SIZE));
        }
        
        void decode(const void* data,
                    size_t bufferSize,
                    iTC::Payload::Ack &ack) {
            
            // Check paquet size
            if (bufferSize != ITC_ACK_SIZE) {
                throw std::runtime_error("Packet has invalid size");
            }
            
            // Read count
            ack._count = *((uint8_t*) data);
            
            // Read code
            ack._code = (iTC::AckCode) ((uint8_t*) data)[ITC_PAYLOAD_ACK_CODE_OFFSET];
            
        }
        
        void decode(const void* data,
                    iTC::Header &header) {
            
            // Read payload type
            header._payloadType = (iTC::PayloadType) *((uint8_t*) data);
            
            // Read message Id
            header._messageId = ntohl(*((uint32_t*) &((uint8_t*) data)[ITC_HEADER_MESSAGE_ID_OFFSET]));
            
        }
        
        void encode(void* target,
                    const iTC::Header &header) {
            
            // Copy payload type
            uint8_t payloadType = static_cast<uint8_t>(header._payloadType);
            memcpy(target, &payloadType, ITC_HEADER_PAYLOAD_TYPE_SIZE);
            
            // Copy message ID
            uint32_t messageId = htonl(header._messageId);
            memcpy(&(((uint8_t*) target)[ITC_HEADER_MESSAGE_ID_OFFSET]), &messageId, ITC_HEADER_MESSAGE_ID_SIZE);
        }
        
        void encode(void* target,
                    const iTC::Payload::Ack &payload) {
            
            // Copy count field
            memcpy(target, &payload._count, ITC_PAYLOAD_ACK_COUNT_SIZE);
            
            // Copy ack code
            uint8_t code = static_cast<uint8_t>(payload._code);
            memcpy(&(((uint8_t*) target)[ITC_PAYLOAD_ACK_CODE_OFFSET]), &code, ITC_PAYLOAD_ACK_CODE_SIZE);
        }
        
        void encode(void* target,
                    const iTC::Payload::Message &payload) {
            
            // Copy count field
            memcpy(target, &payload._count, ITC_PAYLOAD_MESSAGE_COUNT_SIZE);
            
            // Copy body
            memcpy(&(((uint8_t*) target)[ITC_PAYLOAD_MESSAGE_BODY_OFFSET]), boost::asio::buffer_cast<const uint8_t*>(payload._body), boost::asio::buffer_size(payload._body));
        }
        
        void encode(void* target,
                    const iTC::Ack &ack) {
            encode(target, ack._header);
            encode(ITC_GET_PAYLOAD(target), ack._payload);
        }
        
        void encode(void* target,
                    const iTC::Message &message) {
            encode(target, message._header);
            encode(ITC_GET_PAYLOAD(target), message._payload);
        }
        
    }
    
    // ReliableAdapter::HandledMessage
    
    ReliableAdapter::HandledMessage::HandledMessage(const iTC::Message &message,
                                                    iTC::AckCode code) :
    
    _messageId  (message._header._messageId),
    _ackCode    (code),
    _count      (0)
    
    { }
    
    iTC::Ack ReliableAdapter::HandledMessage::makeAck() {
        
        // Return new ack
        
        return iTC::Ack(iTC::Header( { iTC::PayloadType::Ack, _messageId } ), iTC::Payload::Ack( { _count++, _ackCode } ) );
    }
    
    // ReliableTask
    
    struct ReliableTask : public std::enable_shared_from_this<ReliableTask> {
        
        // Init
        
        ReliableTask(boost::asio::io_service &ioService,
                     const Buffer &body,
                     iTC::MessageId messageId) :
        
        _finished       (false),
        _message        ( { 0, messageId, body } ),
        _timerMsg       (ioService),
        _timerGlobal    (ioService)
        
        { }
        
        // Clean object
        
        void clean() {
            
            // Cancel timers
            _timerMsg.cancel();
            _timerGlobal.cancel();
            
            // Clear connections
            _connections.clear();
            
        }
        
        // Encode message
        
        iTC::Message makeMessage() {
            return iTC::Message(iTC::Header( { iTC::PayloadType::Message, _message._id } ), iTC::Payload::Message( { _message._count++, _message._body } ) );
        }
        
        // Timer callback
        
        void onMsgTimerCallback(const boost::system::error_code &error) {
            
            // Note : Call by worker
            
            if (!error && !_finished) {
                
                // Timeout !
                
                // Encode message
                auto message = makeMessage();
                
                // Rearm timer
                _timerMsg.expires_from_now(boost::posix_time::microseconds(_adapterPtr->timeoutFunc(message._payload._count)));
                _timerMsg.async_wait(_adapterPtr->_strand.wrap(std::bind(&ReliableTask::onMsgTimerCallback, shared_from_this(), std::placeholders::_1)));
                
                // Send the message (again)
                _adapterPtr->send(message, nullptr);
            }
            
        }
        
        // Attributes
        
        // Common task attributes
        
        bool _finished;
        WriteCallback _handler;
        std::vector<boost::signals2::scoped_connection> _connections;
        
        // Message attributes
        
        struct {
            
            uint8_t                     _count;
            const iTC::MessageId        _id;
            const Buffer                _body;
            
        } _message;
        
        // Timers
        
        boost::asio::deadline_timer     _timerMsg;
        boost::asio::deadline_timer     _timerGlobal;
        
        // Shared pointers
        
        ReliableAdapter::Ptr            _adapterPtr;
        
    };
    
    // ReliableAdapter
    
    ReliableAdapter::ReliableAdapter(boost::asio::io_service &ioService,
                                     const Config &config) :
    
    _strand     (ioService),
    _config     (config),
    _state      (Stopped),
    _messageId  (0)
    
    { }
    
    void ReliableAdapter::init(const Adapter::Callbacks &callbacks) {
        
        auto ptr = shared_from_this();
        _strand.dispatch([ptr, callbacks]() {
            
            // Note : Call by worker
            
            // Check state
            if (ptr->_state != Stopped) {
                return;
            }
            
            // Save user callbacks
            ptr->_callbacks = callbacks;
            
            // Update state
            ptr->_state = Started;
            
        });
    }
    
    void ReliableAdapter::cancel() {
        auto ptr = shared_from_this();
        _strand.dispatch([ptr]() {
            ptr->disconnect();
        });
    }
    
    void ReliableAdapter::handleIncommingData(const Buffer &buffer,
                                              const WriteCallback &writeCallback) {
        _strand.dispatch(std::bind(&ReliableAdapter::handleIncommingData_internal, shared_from_this(),
                                   buffer,
                                   writeCallback));
        
    }
    
    void ReliableAdapter::handleOutgoingData(const Buffer &buffer,
                                             const WriteCallback &writeCallback) {
        _strand.dispatch(std::bind(&ReliableAdapter::handleOutgoingData_internal, shared_from_this(),
                                   buffer,
                                   writeCallback));
    }
    
    void ReliableAdapter::handleIncommingData_internal(const Buffer &buffer,
                                                       const WriteCallback &writeCallback) {
        
        // Note :: Call by worker
        
        if (_state != Started) {
            auto error = std::make_exception_ptr(std::runtime_error(std::string("Method [") +
                                                                    __PRETTY_FUNCTION__ +
                                                                    "] : Invalid state (" +
                                                                    std::to_string(_state) + ")"));
            
            if (writeCallback) {
                writeCallback(error);
            }
            
            return;
        }
        
        std::exception_ptr error = nullptr;
        
        size_t bufferSize = boost::asio::buffer_size(buffer);
        
        // First check packet size
        if (bufferSize < ITC_MESSAGE_MINIMAL_SIZE) {
            error = std::make_exception_ptr(std::runtime_error("Invalid packet size"));
            if (writeCallback) {
                writeCallback(error);
            }
            return;
        }
        
        auto data = boost::asio::buffer_cast<const void*>(buffer);
        
        // First decode header
        iTC::Header header;
        decode(data, header);
        
        switch (header._payloadType) {
                
                /* iTC::PayloadType::Message */
            case iTC::PayloadType::Message:
            {
                iTC::Payload::Message payload;
                
                try {
                    iTC::decode(ITC_GET_PAYLOAD(data), bufferSize, payload);
                } catch (...) {
                    error = std::current_exception();
                }
                
                if (!error) {
                    onIncommingMessage(iTC::Message(header, payload),
                                       writeCallback);
                }
                
            }
                /* iTC::PayloadType::Message */
                break;
                
                /* iTC::PayloadType::Ack */
            case iTC::PayloadType::Ack:
            {
                iTC::Payload::Ack payload;
                
                try {
                    iTC::decode(ITC_GET_PAYLOAD(data), bufferSize, payload);
                } catch (...) {
                    error = std::current_exception();
                }
                
                if (!error) {
                    onIncommingAck(iTC::Ack(header, payload));
                    
                    if (writeCallback) {
                        writeCallback(nullptr);
                    }
                }
                
            }
                /* iTC::PayloadType::Ack */
                break;
                
            default:
                error = std::make_exception_ptr(std::runtime_error("Invalid payload type"));
        }
        
        if (error) {
            if (writeCallback) {
                writeCallback(error);
            }
        }
    }
    
    void ReliableAdapter::handleOutgoingData_internal(const Buffer &buffer,
                                                      const WriteCallback &writeCallback) {
        
        // Note : Call by worker
        
        if (_state != Started) {
            auto error = std::make_exception_ptr(std::runtime_error(std::string("Method [") +
                                                                    __PRETTY_FUNCTION__ +
                                                                    "] : Invalid state (" +
                                                                    std::to_string(_state) + ")"));
            
            if (writeCallback) {
                writeCallback(error);
            }
            
            return;
        }
        
        auto messageId = getMessageId();
        auto taskPtr = std::make_shared<ReliableTask>(_strand.get_io_service(), buffer, messageId);
        
        taskPtr->_handler       = writeCallback;
        taskPtr->_adapterPtr    = shared_from_this();
        
        // Disconnection callback
        
        auto onDisconnection = [taskPtr]() {
            
            // Note : Call by worker
            
            if (!taskPtr->_finished) {
                
                auto reason = std::make_exception_ptr(std::runtime_error("Reliable adapter stopped"));
                
                taskPtr->_finished = true;
                if (taskPtr->_handler) {
                    taskPtr->_handler(reason);
                }
                taskPtr->clean();
            }
        };
        
        // Abortion callback
        
        auto onAbortion = [taskPtr, messageId](iTC::MessageId idToCancel) {
            
            // Note : Call by worker
            
            if (messageId == idToCancel) {
                
                if (!taskPtr->_finished) {
                    
                    auto reason = std::make_exception_ptr(std::runtime_error("Operation cancelled"));
                    
                    taskPtr->_finished = true;
                    if (taskPtr->_handler) {
                        taskPtr->_handler(reason);
                    }
                    taskPtr->clean();
                    
                    return true;
                }
            }
            
            return false;
        };
        
        // Acknowledgment callback
        
        auto onAcknowledgment = [taskPtr, messageId](const iTC::Ack &ack) {
            
            // Note : Call by worker
            
            if (ack._header._messageId == messageId) {
                
                if (!taskPtr->_finished) {
                    
                    taskPtr->_finished = true;
                    if (taskPtr->_handler) {
                        taskPtr->_handler(nullptr);
                    }
                    taskPtr->clean();
                    
                }
                
                return true;
            }
            
            return false;
        };
        
        // Timer callback
        
        auto timerGlobalCallback = [taskPtr](const boost::system::error_code &error) {
            
            // Note : Call by worker
            
            if (!error && !taskPtr->_finished) {
                
                auto reason = std::make_exception_ptr(std::runtime_error("Request timeout"));
                
                taskPtr->_finished = true;
                if (taskPtr->_handler) {
                    taskPtr->_handler(reason);
                }
                taskPtr->clean();
            }
        };
        
        // Subscribe disconnection callbacks
        taskPtr->_connections.push_back(_disconnection_Signal.connect(onDisconnection));
        
        // Subscribe abortion callbacks
        taskPtr->_connections.push_back(_cancel_Signal.connect(onAbortion));
        
        // Subscribe acknowledgment callbacks
        taskPtr->_connections.push_back(_acks_Signal.connect(onAcknowledgment));
        
        // Encode message
        auto message = taskPtr->makeMessage();
        
        // Start message timer
        taskPtr->_timerMsg.expires_from_now(boost::posix_time::microseconds(timeoutFunc(message._payload._count)));
        taskPtr->_timerMsg.async_wait(_strand.wrap([taskPtr](const boost::system::error_code &error) {
            taskPtr->onMsgTimerCallback(error);
        }));
        
        // Start global timer
        taskPtr->_timerGlobal.expires_from_now(boost::posix_time::microseconds(_config._globalTimeout));
        taskPtr->_timerGlobal.async_wait(_strand.wrap(timerGlobalCallback));
        
        // Finaly send the message !
        send(message, nullptr);
        
    }
    
    void ReliableAdapter::send(std::shared_ptr<uint8_t> buffer,
                               size_t bufferSize,
                               const WriteCallback &writeCallback) {
        
        // Note : (Must be) call by worker
        
        _callbacks._onOutgoingData(Buffer(buffer.get(), bufferSize), [writeCallback, buffer](std::exception_ptr error) {
            if (writeCallback) {
                writeCallback(error);
            }
        });
    }
    
    void ReliableAdapter::send(const iTC::Message &message,
                               const WriteCallback &writeCallback) {
        
        // Note : (Must be) call by worker
        
        // Allocate memory
        size_t bufferSize = ITC_MESSAGE_MINIMAL_SIZE + boost::asio::buffer_size(message._payload._body);
        auto buffer = std::shared_ptr<uint8_t>(new uint8_t[bufferSize], std::default_delete<uint8_t[]>());
        
        // Encode data (Will copy the body)
        encode(buffer.get(), message);
        
        // Send data
        send(buffer, bufferSize, writeCallback);
        
    }
    
    void ReliableAdapter::send(const iTC::Ack &ack,
                               const WriteCallback &writeCallback) {
        
        // Note : (Must be) call by worker
        
        // Allocate memory
        size_t bufferSize = ITC_ACK_SIZE;
        auto buffer = std::shared_ptr<uint8_t>(new uint8_t[bufferSize], std::default_delete<uint8_t[]>());
        
        // Encode data
        encode(buffer.get(), ack);
        
        // Send data
        send(buffer, bufferSize, writeCallback);
        
    }
    
    uint64_t ReliableAdapter::defaultTimeoutFunc(size_t) {
        return 100000; /* 100 Milliseconds */
    }
    
    uint64_t ReliableAdapter::timeoutFunc(size_t count) {
        if (_config._timeoutFunc) {
            return _config._timeoutFunc(count);
        } else {
            return defaultTimeoutFunc(count);
        }
    }
    
    void ReliableAdapter::onIncommingMessage(const iTC::Message &message,
                                             const WriteCallback &writeCallback) {
        
        // TODO : Add fixed size map
        
        auto messageId = message._header._messageId;
        
        auto iterator = _handledMessages.find(messageId);
        if (iterator == _handledMessages.end()) {
            
            // Create HandledMessage for this message
            auto handledMsg = std::make_shared<HandledMessage>(message, iTC::AckCode::Ok);
            _handledMessages.insert(std::make_pair(messageId, handledMsg));
            iterator = _handledMessages.find(messageId);
            
            // Forward message to user
            _callbacks._onIncommingMessage(message._payload._body, writeCallback);
            
        } else {
            if (writeCallback) {
                writeCallback(nullptr);
            }
        }
        
        send(iterator->second->makeAck(), nullptr);
    }
    
    bool ReliableAdapter::onIncommingAck(const iTC::Ack &ack) {
        
        // Note : (Must be) Call by worker
        
        return _acks_Signal(ack);
    }
    
    bool ReliableAdapter::cancel(iTC::MessageId messageId) {
        
        // Note : (Must be) Call by worker
        
        return _cancel_Signal(messageId);
    }
    
    iTC::MessageId ReliableAdapter::getMessageId() {
        
        unsigned int result = _messageId.fetch_add(1);
        if (result != 0) {
            return result;
        } else {
            return getMessageId();
        }
    }
    
    void ReliableAdapter::disconnect() {
        
        // Note : Call by worker
        
        // Check state
        if (_state != Started) {
            return;
        }
        
        // Cancel tasks
        _disconnection_Signal();
        
        // Unereference user callbacks
        _callbacks = Callbacks();
        
        // Clear active messgae ID
        _messageId = 0;
        
        // Clear handled messgaes map
        _handledMessages.clear();
        
        // Update state
        _state = Stopped;
    }
    
} }
