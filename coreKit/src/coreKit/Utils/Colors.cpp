//
//  colors.cpp
//  coreKit
//
//  Created by Maxime Nuss on 02/11/17.
//
//

#include "Colors.hpp"

namespace coreKit { namespace Color {

    // Formatting codes
    const std::string reset      = "\033[00m";
    const std::string bold       = "\033[1m";
    const std::string dark       = "\033[2m";
    const std::string underline  = "\033[4m";
    const std::string blink      = "\033[5m";
    const std::string reverse    = "\033[7m";
    const std::string concealed  = "\033[8m";

    // Foreground colors
    const std::string grey       = "\033[30m";
    const std::string red        = "\033[31m";
    const std::string green      = "\033[32m";
    const std::string yellow     = "\033[33m";
    const std::string blue       = "\033[34m";
    const std::string magenta    = "\033[35m";
    const std::string cyan       = "\033[36m";
    const std::string white      = "\033[37m";

    // Background colors
    const std::string on_grey    = "\033[40m";
    const std::string on_red     = "\033[41m";
    const std::string on_green   = "\033[42m";
    const std::string on_yellow  = "\033[43m";
    const std::string on_blue    = "\033[44m";
    const std::string on_magenta = "\033[45m";
    const std::string on_cyan    = "\033[46m";
    const std::string on_white   = "\033[47m";

} }
