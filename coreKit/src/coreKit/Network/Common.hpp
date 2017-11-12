//
//  Common.hpp
//  uavia-embedded
//
//  Created by Pierre Pelé on 23/10/2017.
//

#pragma once

#include <exception>
#include <functional>

#include <boost/asio/buffer.hpp>

namespace coreKit { namespace Network {
    
    // Declarations
    
    using Buffer            = boost::asio::const_buffer;
    using WriteCallback     = std::function<void(const std::exception_ptr)>;
    using OnReadError       = std::function<void(const std::exception_ptr)>;
    
    using OnIncommingData   = std::function<void(const Buffer&,
                                                 const WriteCallback&)>;
    using OnOutgoingData    = std::function<void(const Buffer&,
                                                 const WriteCallback&)>;
    
} }
