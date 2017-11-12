//
//  colors.hpp
//  coreKit
//
//  Created by Maxime Nuss on 02/11/17.
//
//

#pragma once

#include <string>

namespace coreKit { namespace Color {

    // Formatting codes
    extern const std::string reset;
    extern const std::string bold;
    extern const std::string dark;
    extern const std::string underline;
    extern const std::string blink;
    extern const std::string reverse;
    extern const std::string concealed;

    // Foreground colors
    extern const std::string grey;
    extern const std::string red;
    extern const std::string green;
    extern const std::string yellow;
    extern const std::string blue;
    extern const std::string magenta;
    extern const std::string cyan;
    extern const std::string white;

    // Background colors
    extern const std::string on_grey;
    extern const std::string on_red;
    extern const std::string on_green;
    extern const std::string on_yellow;
    extern const std::string on_blue;
    extern const std::string on_magenta;
    extern const std::string on_cyan;
    extern const std::string on_white;

} }
