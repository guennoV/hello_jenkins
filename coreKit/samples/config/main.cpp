//
//  main.cpp
//  uavia-software-next
//
//

#include <iostream>

#include <coreKit/Config/Config.hpp>

const coreKit::ConfigList configList_1({ "Video", "TestSource" }, {
    
    coreKit::makeParam<unsigned int>("Width"         , "Width in pixel"          , 640    ),
    coreKit::makeParam<unsigned int>("Height"        , "Height in pixel"         , 480    )
    
});

const coreKit::ConfigList configList_2({ "Video", "TestSource" }, {
    
    coreKit::makeParam<unsigned int>("Framerate"     , "Framerate in fps"        , 24     )
    
});

int main() {
    
    {
        // Method N°1 : Use values directly
        std::cout<<"Method 1 : \n";
        std::cout << "Width     (pixel)  : " << std::to_string(coreKit::Config::get<unsigned int>({ { "Video", "TestSource" }, "Width" })) << std::endl;
        std::cout << "Width     (pixel)  : " << std::to_string(coreKit::Config
                                                               ::get<unsigned int>("Video.TestSource.Width")) << std::endl;
        
        coreKit::Config::set<unsigned int>({ { "Video", "TestSource" }, "Width" }, 1920);
        coreKit::Config::set<unsigned int>("Video.TestSource.Width", 1920);
        
        std::cout << "Width     (pixel)  : " << std::to_string(coreKit::Config::get<unsigned int>({ { "Video", "TestSource" }, "Width" })) << std::endl;
        std::cout << "Width     (pixel)  : " << std::to_string(coreKit::Config::get<unsigned int>("Video.TestSource.Width")) << std::endl;
    }
    
    {
        // Method N°2 : Use pointer to parameter
        std::cout<<"\n\nMethod 2 : \n";
        
        auto height = coreKit::Config::getParam<unsigned int>("Video.TestSource.Height");
        
        std::cout << "Height    (pixel)  : " << std::to_string(height->get()) << std::endl;
        
        height->set(1080);
        
        std::cout << "Height    (pixel)  : " << std::to_string(height->get()) << std::endl;
    }
    
    {
        // Method N°3 : Only use strings !
        std::cout<<"\n\nMethod 3 : \n";
        std::cout << "Framerate (fps)    : " << coreKit::Config::get_generic({ { "Video", "TestSource" }, "Framerate" }) << std::endl;
        std::cout << "Framerate (fps)    : " << coreKit::Config::get_generic("Video.TestSource.Framerate") << std::endl;
        
        coreKit::Config::set_generic({ { "Video", "TestSource" }, "Framerate" }, "30");
        coreKit::Config::set_generic("Video.TestSource.Framerate", "30");
        
        std::cout << "Framerate (fps)    : " << coreKit::Config::get_generic({ { "Video", "TestSource" }, "Framerate" }) << std::endl;
        std::cout << "Framerate (fps)    : " << coreKit::Config::get_generic("Video.TestSource.Framerate") << std::endl;
    }
    
    // Exit
    
    return EXIT_SUCCESS;
}
