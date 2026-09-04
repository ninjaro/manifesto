#pragma once

#include "cli/common_args.hpp"

#include <optional>
#include <string>

namespace ecosystem::engels_cli_support {

struct engels_request {
    std::string command;
    args_list command_args;
};

std::string engels_usage_text();

std::optional<engels_request> parse_engels_request(
    const args_list& args, std::string* error_message
);

}  // namespace ecosystem::engels_cli_support
