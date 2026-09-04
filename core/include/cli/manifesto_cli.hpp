#pragma once

#include <iosfwd>
#include <string>
#include <vector>

namespace ecosystem {

int run_manifesto(int argc, const char* const* argv);

int run_manifesto_cli(
    const std::vector<std::string>& args, std::ostream& out, std::ostream& err
);

}  // namespace ecosystem
