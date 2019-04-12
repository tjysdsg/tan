#include <boost/program_options.hpp>
#include <iostream>
#include "config.h"

namespace po = boost::program_options;
int main(int argc, char **argv) {
    // Declare the supported options.
    po::options_description desc("Allowed options");
    desc.add_options()("help", "produce help message")(
        "version,v", "version of current program");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    if (vm.count("version")) {
        std::cout << "tanc version: " << TAN_VERSION_MAJOR << "."
                  << TAN_VERSION_MINOR << "." << TAN_VERSION_PATCH
                  << "\n";
        return 0;
    }

    return 0;
}
