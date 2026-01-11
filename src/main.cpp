#include "cli_options.h"

int main(int argc, char* argv[]) {
    auto opts = NParallelGrep::NCli::ParseCli(argc, argv);
    return 0;
}
