#pragma once

#include "packages/package_dependency.hpp"
#include "manifest.hpp"

#include <optional>
#include <vector>

namespace ecosystem {

dependency_summary summarize_dependencies(
    const manifest& manifest_value, const std::optional<artifact_ref>& requested_artifact
);
bool dependency_summary_has_entries(const dependency_summary& dependencies);
std::vector<std::string> component_package_link_targets(const component& component_value);

}  // namespace ecosystem
