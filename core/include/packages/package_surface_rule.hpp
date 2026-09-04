#pragma once

#include "packages/package_dependency_id.hpp"
#include "packages/package_surface_block.hpp"

#include <optional>
#include <string_view>
#include <utility>
#include <vector>

namespace ecosystem {

struct package_surface_options;

enum class package_surface_source {
    static_rule,
    component_rule,
    profile_rule,
};

struct package_surface_target_rule {
    std::string_view name;
    std::vector<std::string_view> link_libraries;
    std::vector<std::string_view> include_directories;
    std::string_view condition;
    bool system_include_directories = false;
};

struct package_surface_component_rule {
    std::optional<dependency_id> dependency;
    std::vector<std::string_view> pre_find_package_lines;
    std::string_view find_package_open_line;
    std::string_view component_line_prefix = "        ";
    std::string_view find_package_close_line = ")";
    std::vector<std::string_view> post_find_package_lines;
    std::vector<std::string_view> qt_automation_lines;
    bool blank_line_before_qt_automation = false;
};

struct package_surface_conditional_line_rule {
    std::optional<dependency_id> dependency;
    std::string_view line;
};

struct package_surface_profile_rule {
    std::vector<std::string_view> option_lines;
    std::string_view condition;
    std::optional<dependency_id> component_dependency;
    std::string_view component_find_package_open_line;
    std::string_view component_line_prefix = "        ";
    std::string_view component_find_package_close_line = ")";
    std::string_view component_find_package_line_prefix;
    std::string_view component_find_package_line_suffix;
    std::vector<std::pair<std::string_view, std::string_view>>
        component_find_package_value_aliases;
    std::vector<package_surface_conditional_line_rule> fixed_find_package_lines;
    std::string_view support_target_name;
    std::optional<dependency_id> support_component_dependency;
    std::string_view support_component_line_prefix = "        ";
    std::string_view support_component_value_prefix;
    std::vector<package_surface_conditional_line_rule> fixed_support_link_lines;
};

struct package_surface_rule {
    package_surface_source source = package_surface_source::static_rule;
    std::vector<std::string_view> find_package_lines;
    std::vector<std::string_view> pre_target_lines;
    package_surface_component_rule component_rule;
    package_surface_profile_rule profile_rule;
    package_surface_target_rule support_target;
    bool blank_line_before_target = false;
};

const package_surface_rule*
find_package_surface_rule(package_surface_block block);

} // namespace ecosystem
