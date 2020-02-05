#include "config.h"
#include <boost/program_options.hpp>
#include <iostream>

namespace po = boost::program_options;

int main(int argc, char **argv) {
    // Declare the supported options.
    po::options_description desc("Allowed options");
    desc.add_options()("help", "produce help message")("version,v", "version of current program")(
        "files", po::value<std::vector<std::string>>(), "Input file for compiling");
    // positional option file
    po::positional_options_description p;
    p.add("files", -1);
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    po::notify(vm);
    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }
    if (vm.count("version")) {
        std::cout << "tanc version: " << TAN_VERSION_MAJOR << "." << TAN_VERSION_MINOR << "." << TAN_VERSION_PATCH
                  << "\n";
        return 0;
    }
    if (vm.count("files")) {
        std::vector<std::string> files = vm["files"].as<std::vector<std::string>>();
        for (const std::string &file : files) {
            // TODO: call parser to parse files
        }
    }
    return 0;
}
