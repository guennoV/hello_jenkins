//
//  main.cpp
//  uavia-software-next
//
//

#include <iostream>

#include <boost/program_options.hpp>

#include <coreKit/Log/Log.hpp>
#include <coreKit/Config/Config.hpp>

const coreKit::ConfigList configList({ "GeoKit", "Photo" }, {
    
    coreKit::makeParam<unsigned int>("Width"         , "Width in pixel"          , 640    ),
    coreKit::makeParam<unsigned int>("Height"        , "Height in pixel"         , 480    )
    
});

int main(int argc, const char*argv[]) {
    
    std::string filename;
    
    // Parse command line arguments
    
    {
        namespace po        = boost::program_options;
        namespace po_style  = boost::program_options::command_line_style;
        
        po::options_description desc { "Options :" };
        
        // Options definition
        
        desc.add_options()
        ("help,h", "Display this help screen")
        ("config,c", po::value<std::string>(), "YAML configuration file");
        
        // Boost program options initialization
        
        po::variables_map vm;
        
        {
            po::store(po::command_line_parser(argc, argv).options(desc).style(po_style::unix_style | po_style::case_insensitive).run(), vm);
            po::store(parse_command_line(argc, argv, desc), vm);
            po::notify(vm);
            
            if (vm.count("help")) {
                std::cout << desc << std::endl;
                
                return EXIT_SUCCESS;
            }
            
            if (vm.count("config")) {
                filename = vm["config"].as<std::string>();
            }
        }
    }
    
    // Add log sink
    
    {
        spdlog::sink_ptr sink = std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>();
        coreKit::Log::subscribe(sink);
    }
    
    // Parse YAML file if any
    
    if (filename.size() > 0) {
        coreKit::Config::parseYaml(filename);
    }
    
    // Parse environment
    
    coreKit::Config::parseEnv();
    
    // Print params
    
    std::cout << "Width     (pixel)  : " << coreKit::Config::get_generic("GeoKit.Photo.Width"    ) << std::endl;
    std::cout << "Height    (pixel)  : " << coreKit::Config::get_generic("GeoKit.Photo.Height"   ) << std::endl;
    
    // Print full config
    
    std::cout << std::endl << coreKit::Config::getAppConfig().toString() << std::endl;
    
    // Exit
    
    return EXIT_SUCCESS;
}
