//
//  StackTrace.cpp
//  uavia-software-next
//
//  Created by Pierre Pelé on 17/03/17.
//
//

#include "StackTrace.hpp"

#include <sstream>

#ifndef BACKTRACE_FRAMES
#define BACKTRACE_FRAMES            32
#endif

#ifndef BACKTRACE_FRAME_OFFSET
#define BACKTRACE_FRAME_OFFSET      2
#endif

#ifdef HAVE_CONFIG_H
#include <coreKit_config.h>
#endif

#if defined(BACKTRACE_USE_BACKWARD)
#include <backward.hpp>
#elif defined(BACKTRACE_USE_BOOSTER)
#include <booster/backtrace.h>
#endif

namespace coreKit {
    
    // StackTrace
    
    void handleStackTrace() {
        
#if defined(BACKTRACE_USE_BACKWARD)
        backward::SignalHandling();
#else
        // throw std::runtime_error("No StackTrace support");
#endif
        
    }
    
    std::string getBackTrace() {
        
#if defined(BACKTRACE_USE_BACKWARD)
        backward::StackTrace st;
        st.load_here(BACKTRACE_FRAMES);
        std::ostringstream result;
        backward::TraceResolver  resolver;
        result << "Trace ";
        unsigned int const thread_id = st.thread_id();
        if (thread_id) {
            result << "in thread N°" << thread_id << " :" << std::endl;
        } else {
            result << ":" << std::endl;
        }
        
        resolver.load_stacktrace(st);
        for (size_t i = BACKTRACE_FRAME_OFFSET; i < st.size(); ++i) {
            
            backward::ResolvedTrace const & trace = resolver.resolve(st[i]);
            bool indented = true;
            
            result << "#" <<
            std::setfill(' ') << std::setw(2) << std::left <<
            std::dec << (trace.idx - BACKTRACE_FRAME_OFFSET) << " ";
            if (trace.source.filename.empty()) {
                result <<
                std::setfill(' ') << std::setw(18) << std::right <<
                std::hex << trace.addr << " in " <<
                trace.object_function << std::endl <<
                "   at " << trace.object_filename << std::endl;
                indented = false;
            }
            
            for (size_t j = 0; j < trace.inliners.size(); ++j) {
                if (not indented) {
                    result << "    ";
                }
                
                backward::ResolvedTrace::SourceLoc const & location =
                trace.inliners[j];
                result <<
                "     (inlined)     "
                "in " << location.function << std::endl <<
                "   at " << location.filename << ":" <<
                std::dec << location.line << std::endl;
                indented = false;
            }
            
            if (not trace.source.filename.empty()) {
                
                if (not indented) {
                    result << "    ";
                }
                
                result <<
                std::setfill(' ') << std::setw(18) << std::right <<
                std::hex << trace.addr << " in " <<
                trace.source.function << std::endl <<
                "   at " << trace.source.filename << ":" <<
                std::dec << trace.source.line << std::endl;
            }
        }
        
        return result.str();
        
#elif defined(BACKTRACE_USE_BOOSTER)
        booster::backtrace b(BACKTRACE_FRAMES);
        std::ostringstream result;
        result << "Trace :" << std::endl;
        for (unsigned int i = BACKTRACE_FRAME_OFFSET; i < b.stack_size(); ++i) {
            result << "#" <<
            std::setfill(' ') << std::setw(2) << std::left <<
            std::dec << (i - BACKTRACE_FRAME_OFFSET) << " ";
            b.trace_line(i, result);
        }
        
        return result.str();
#else
        return std::string("Trace is unknown (No BackTrace support)");
#endif
        
    }
    
}
