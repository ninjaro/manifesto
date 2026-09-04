#pragma once

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

namespace ecosystem {

using template_bindings = std::vector<std::pair<std::string, std::string>>;

std::filesystem::path template_root_path();
std::filesystem::path
locate_template_path(const std::vector<std::filesystem::path> &relative_paths);
std::string render_text_template(const std::filesystem::path &relative_path,
                                 const template_bindings &bindings,
                                 std::string *error_message);
std::string render_text_template_candidates(
    const std::vector<std::filesystem::path> &relative_paths,
    const template_bindings &bindings, std::string *error_message);

} // namespace ecosystem
