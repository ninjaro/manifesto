#pragma once

#include <nlohmann/json.hpp>

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace ecosystem {

using json = nlohmann::ordered_json;
using string_list = std::vector<std::string>;

struct artifact_ref {
    std::string component_id;
    std::string artifact_id;
};

struct artifact {
    std::string id;
    std::string kind;
    std::string name;
    string_list link;
};

struct file_unit {
    std::string id;
    std::string kind;
};

struct component {
    std::string id;
    std::string description;
    std::string root;
    json stack = json::object();
    json tests = json::object();
    json benchmarks = json::object();
    std::vector<std::string> modules;
    std::vector<artifact> artifacts;
    std::vector<file_unit> file_units;
};

struct manifest {
    std::string id;
    std::string description;
    std::string version;
    int cpp_standard = 20;
    std::string android_application_id;
    std::string android_package_source_dir;
    bool install_assets = false;
    std::string facade_entry_artifact;
    std::vector<component> components;
    json relations = json::array();
};

struct manifest_report {
    std::filesystem::path path;
    std::optional<manifest> value;
    string_list errors;
    bool has_manifest = false;
};

manifest_report load_manifest(const std::filesystem::path& manifest_path);
bool save_manifest(
    const std::filesystem::path& manifest_path, const manifest& value,
    std::string* error_message
);

string_list validate_manifest(const manifest& value);
json to_json(const manifest& value);

std::optional<artifact_ref> parse_artifact_ref(const std::string& value);
std::string format_artifact_ref(const artifact_ref& value);

} // namespace ecosystem
