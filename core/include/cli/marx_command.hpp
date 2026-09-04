#pragma once

#include "cli/common_args.hpp"

#include <iosfwd>

namespace ecosystem {

int run_marx_command(
    const args_list& args, std::ostream& out, std::ostream& err
);

}  // namespace ecosystem
