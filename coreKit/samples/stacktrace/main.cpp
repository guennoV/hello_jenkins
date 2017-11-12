//
//  main.cpp
//  uavia-software-next
//
//

#include <sys/resource.h>

#include <iostream>
#include <string>
#include <thread>

#include <coreKit/Utils/StackTrace.hpp>

int you_shall_not_pass() {
    
    char* ptr = (char*) 42;
    int v = *ptr;
    
    return v;
}

void abort_abort_I_repeat_abort_abort() {
    
    std::cout << "Jumping off the boat !" << std::endl;
    
    abort();
}

volatile int zero = 0;

int divide_by_zero() {
    
    std::cout << "And the wild black hole appears ..." << std::endl;
    
    int v = 42 / zero;
    return v;
}

#ifdef __TESTING_STACK_OVERFLOW__
int bye_bye_stack(int i) {
    return bye_bye_stack(i + 1) + bye_bye_stack(i * 2);
}

void stack_overflow() {
    
    struct rlimit limit;
    limit.rlim_max = 8096;
    setrlimit(RLIMIT_STACK, &limit);
    
    int result = bye_bye_stack(42);
    std::cout << "Result : " << result << std::endl;
}
#endif // __TESTING_STACK_OVERFLOW__

void superFunction() {
    
    std::string* stringPtr = nullptr;
    stringPtr->append("Hello world !");
}

int main() {
    
    // Add StackTrace ability
    
    coreKit::handleStackTrace();
    
    // Time to code
    
    // you_shall_not_pass();
    // abort_abort_I_repeat_abort_abort();
    // divide_by_zero();
    
#ifdef __TESTING_STACK_OVERFLOW__
    // stack_overflow();
#endif // __TESTING_STACK_OVERFLOW__
    
    std::thread thread([]() {
        superFunction();
    });
    
    thread.join();
    
    // Exit
    
    return EXIT_SUCCESS;
}
