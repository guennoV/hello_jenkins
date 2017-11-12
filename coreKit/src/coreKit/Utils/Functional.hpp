/**
 * @Author: Alexandre Biguet <alexandrebiguet>
 * @Date:   25-Aug-2017
 * @Email:  alexandre.biguet@uavia.eu
 * @Project: Uavia
 * @Filename: Functional.hpp
 * @Last modified by:   alexandrebiguet
 * @Last modified time: 25-Aug-2017
 * @Copyright: Copyright UAVIA © All Rights Reserved.
 */

#pragma once

#include <functional>

namespace coreKit {
    
    // -----------------------------------------------------------------------------
    // Function from lambda
    
    // https://stackoverflow.com/questions/13358672/how-to-convert-a-lambda-to-an-stdfunction-using-templates
    
    template<typename T>
    struct memfun_type {
        using type = void;
    };
    
    template<typename Ret, typename Class, typename... Args>
    struct memfun_type<Ret(Class::*)(Args...) const> {
        using type = std::function<Ret(Args...)>;
    };
    
    template<typename F>
    typename memfun_type<decltype(&F::operator())>::type
    FunctionFromLambda(F const &func) {
        return func;
    }
    
    
    
} // namespace coreKit
