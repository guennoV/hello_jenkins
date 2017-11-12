//
//  memoryCore.hpp
//  coreKit
//
//  Created by Maxime Nuss on 31/08/2017.
//
//

#pragma once

#include <stdint.h>

#include <cstddef>

namespace coreKit {
    
    // Functions prototypes
    
    template<typename T>
    T crc(T crc, const void *data,
          size_t dataLength,
          const T* tab) {
        
        uint8_t *p = (uint8_t*) data;
        crc = crc ^ ~0U;
        
        while (dataLength--) {
            crc = tab[(crc ^ *p++) & 0xFF] ^ (crc >> 8);
        }
        
        return crc ^ ~0U;
    }
    
    uint16_t crc16(uint16_t crc16,
                   const void *data,
                   size_t dataLength,
                   const uint16_t* tab = nullptr);
    
    uint32_t crc32(uint32_t crc32,
                   const void *data,
                   size_t dataLength,
                   const uint32_t* tab = nullptr);
    
    uint64_t crc64(uint64_t crc64,
                   const void *data,
                   size_t dataLength,
                   const uint64_t* tab = nullptr);
    
}
