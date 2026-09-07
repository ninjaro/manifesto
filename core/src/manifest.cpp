#include "manifest.hpp"

#include <algorithm>
#include <fstream>
#include <map>
#include <regex>
#include <set>
#include <sstream>
#include <string>

namespace fs = std::filesystem;

namespace ecosystem {
namespace manifest_support {

    const std::regex id_pattern("^[a-z][a-z0-9_]*$");
    const std::regex android_application_id_pattern(
        "^[A-Za-z][A-Za-z0-9_]*(\\.[A-Za-z][A-Za-z0-9_]*)+$"
    );
    const std::regex android_package_source_dir_pattern(
        "^[A-Za-z0-9._-]+(?:/[A-Za-z0-9._-]+)*$"
    );
    const std::regex project_version_pattern(
        "^[0-9]+\\.[0-9]+\\.[0-9]+(?:[-+][0-9A-Za-z.-]+)?$"
    );
    const std::set<std::string> artifact_kinds {
        "static_lib", "shared_lib", "interface_lib", "exe", "qt_app",
    };
    const std::set<std::string> file_unit_kinds {
        "header_only", "header_only_h", "header_template_impl",
        "source_only", "source_pair_h",
    };

    std::string trim_copy(const std::string& value) {
        const std::size_t first = value.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) {
            return {};
        }
        const std::size_t last = value.find_last_not_of(" \t\r\n");
        return value.substr(first, last - first + 1U);
    }

    std::string normalize_relative_path(const std::string& raw_path) {
        fs::path path = fs::path(trim_copy(raw_path)).lexically_normal();
        std::string normalized = path.generic_string();
        if (normalized == ".") {
            return normalized;
        }
        while (normalized.size() > 1U && normalized.back() == '/') {
            normalized.pop_back();
        }
        if (normalized.rfind("./", 0U) == 0U) {
            normalized.erase(0U, 2U);
        }
        return normalized;
    }

    std::string require_string(
        const json& object, const std::string& field_name,
        const std::string& context, string_list* errors
    ) {
        if (!object.contains(field_name)
            || !object.at(field_name).is_string()) {
            errors->push_back(
                context + "." + field_name + " must be a non-empty string"
            );
            return {};
        }
        const std::string value
            = trim_copy(object.at(field_name).get<std::string>());
        if (value.empty()) {
            errors->push_back(
                context + "." + field_name + " must be a non-empty string"
            );
        }
        return value;
    }

    int require_integer(
        const json& object, const std::string& field_name,
        const std::string& context, string_list* errors
    ) {
        if (!object.contains(field_name)
            || !object.at(field_name).is_number_integer()) {
            errors->push_back(
                context + "." + field_name + " must be an integer"
            );
            return 0;
        }
        return object.at(field_name).get<int>();
    }

    json require_object(
        const json& object, const std::string& field_name,
        const std::string& context, string_list* errors
    ) {
        if (!object.contains(field_name)) {
            errors->push_back(
                context + "." + field_name + " must be a JSON object"
            );
            return json::object();
        }
        if (!object.at(field_name).is_object()) {
            errors->push_back(
                context + "." + field_name + " must be a JSON object"
            );
            return json::object();
        }
        return object.at(field_name);
    }

    bool is_valid_id(const std::string& value) {
        return std::regex_match(value, id_pattern);
    }

    void flatten_modules(
        const json& value, const std::string& prefix,
        const std::string& context, std::vector<std::string>* modules,
        string_list* errors
    ) {
        if (!value.is_array()) {
            errors->push_back(context + " must be an array");
            return;
        }
        for (std::size_t index = 0; index < value.size(); ++index) {
            const json& entry = value.at(index);
            const std::string entry_context
                = context + "[" + std::to_string(index) + "]";
            if (entry.is_string()) {
                const std::string name = normalize_relative_path(
                    prefix + entry.get<std::string>()
                );
                if (name.empty() || name == ".") {
                    errors->push_back(entry_context + " must not be empty");
                    continue;
                }
                modules->push_back(name);
                continue;
            }
            if (entry.is_object() && entry.size() == 1U) {
                const auto iterator = entry.begin();
                const std::string group
                    = normalize_relative_path(iterator.key());
                if (group.empty() || group == ".") {
                    errors->push_back(
                        entry_context + " group key must not be empty"
                    );
                    continue;
                }
                flatten_modules(
                    iterator.value(), prefix + group + "/",
                    entry_context + "." + group, modules, errors
                );
                continue;
            }
            errors->push_back(
                entry_context
                + " must be a string leaf or a single-key object group"
            );
        }
    }

    struct module_node {
        bool leaf = false;
        std::map<std::string, module_node> children;
    };

    void insert_module(module_node* root, const std::string& path) {
        module_node* current = root;
        std::stringstream stream(path);
        std::string segment;
        while (std::getline(stream, segment, '/')) {
            current = &current->children[segment];
        }
        current->leaf = true;
    }

    json emit_module_array(const module_node& root) {
        json output = json::array();
        for (const auto& [name, child] : root.children) {
            if (child.leaf && child.children.empty()) {
                output.push_back(name);
                continue;
            }
            json group = json::object();
            group[name] = emit_module_array(child);
            output.push_back(group);
        }
        return output;
    }

    json modules_to_json(const std::vector<std::string>& modules) {
        module_node root;
        for (const std::string& module_path : modules) {
            insert_module(&root, module_path);
        }
        return emit_module_array(root);
    }

    string_list read_string_list(
        const json& object, const std::string& field_name,
        const std::string& context, string_list* errors
    ) {
        string_list values;
        if (!object.contains(field_name)) {
            return values;
        }
        const json& value = object.at(field_name);
        if (value.is_string()) {
            values.push_back(trim_copy(value.get<std::string>()));
            return values;
        }
        if (!value.is_array()) {
            errors->push_back(
                context + "." + field_name
                + " must be a string or array of strings"
            );
            return values;
        }
        for (std::size_t index = 0; index < value.size(); ++index) {
            if (!value.at(index).is_string()) {
                errors->push_back(
                    context + "." + field_name + "[" + std::to_string(index)
                    + "] must be a string"
                );
                continue;
            }
            values.push_back(trim_copy(value.at(index).get<std::string>()));
        }
        return values;
    }

    artifact parse_artifact(
        const json& object, const std::string& context, string_list* errors
    ) {
        artifact value;
        value.id = require_string(object, "id", context, errors);
        value.kind = require_string(object, "kind", context, errors);
        if (object.contains("name")) {
            value.name = require_string(object, "name", context, errors);
        }
        value.link = read_string_list(object, "link", context, errors);
        return value;
    }

    file_unit parse_file_unit(
        const json& object, const std::string& context, string_list* errors
    ) {
        file_unit value;
        value.id = require_string(object, "id", context, errors);
        value.kind = require_string(object, "kind", context, errors);
        return value;
    }

    component parse_component(
        const json& object, const std::string& context, string_list* errors
    ) {
        component value;
        value.id = require_string(object, "id", context, errors);
        value.description
            = require_string(object, "description", context, errors);
        value.root = normalize_relative_path(
            require_string(object, "root", context, errors)
        );
        if (value.root.empty()) {
            value.root = ".";
        }

        if (object.contains("stack")) {
            if (!object.at("stack").is_object()) {
                errors->push_back(context + ".stack must be a JSON object");
            } else {
                value.stack = object.at("stack");
            }
        }

        if (object.contains("tests")) {
            if (!object.at("tests").is_object()) {
                errors->push_back(context + ".tests must be a JSON object");
            } else {
                value.tests = object.at("tests");
            }
        }

        if (object.contains("benchmarks")) {
            if (!object.at("benchmarks").is_object()) {
                errors->push_back(
                    context + ".benchmarks must be a JSON object"
                );
            } else {
                value.benchmarks = object.at("benchmarks");
            }
        }

        if (!object.contains("modules")) {
            errors->push_back(context + ".modules must be an array");
        } else {
            flatten_modules(
                object.at("modules"), "", context + ".modules", &value.modules,
                errors
            );
        }

        if (!object.contains("artifacts")
            || !object.at("artifacts").is_array()) {
            errors->push_back(context + ".artifacts must be an array");
        } else {
            for (std::size_t index = 0; index < object.at("artifacts").size();
                 ++index) {
                const json& entry = object.at("artifacts").at(index);
                if (!entry.is_object()) {
                    errors->push_back(
                        context + ".artifacts[" + std::to_string(index)
                        + "] must be a JSON object"
                    );
                    continue;
                }
                value.artifacts.push_back(parse_artifact(
                    entry,
                    context + ".artifacts[" + std::to_string(index) + "]",
                    errors
                ));
            }
        }

        if (object.contains("file_units")) {
            if (!object.at("file_units").is_array()) {
                errors->push_back(context + ".file_units must be an array");
            } else {
                for (std::size_t index = 0;
                     index < object.at("file_units").size(); ++index) {
                    const json& entry = object.at("file_units").at(index);
                    if (!entry.is_object()) {
                        errors->push_back(
                            context + ".file_units[" + std::to_string(index)
                            + "] must be a JSON object"
                        );
                        continue;
                    }
                    value.file_units.push_back(parse_file_unit(
                        entry,
                        context + ".file_units[" + std::to_string(index) + "]",
                        errors
                    ));
                }
            }
        }

        return value;
    }

    manifest parse_manifest(const json& root, string_list* errors) {
        manifest value;
        value.id = require_string(root, "id", "manifest", errors);
        value.description
            = require_string(root, "description", "manifest", errors);
        if (root.contains("version")) {
            value.version = require_string(root, "version", "manifest", errors);
        }
        value.cpp_standard
            = require_integer(root, "cpp_standard", "manifest", errors);
        if (root.contains("android_application_id")) {
            value.android_application_id = require_string(
                root, "android_application_id", "manifest", errors
            );
        }
        if (root.contains("android_package_source_dir")) {
            value.android_package_source_dir
                = normalize_relative_path(require_string(
                    root, "android_package_source_dir", "manifest", errors
                ));
        }
        if (root.contains("install_assets")) {
            if (!root.at("install_assets").is_boolean()) {
                errors->push_back("manifest.install_assets must be a boolean");
            } else {
                value.install_assets = root.at("install_assets").get<bool>();
            }
        }

        const json facade = require_object(root, "facade", "manifest", errors);
        value.facade_entry_artifact = require_string(
            facade, "entry_artifact", "manifest.facade", errors
        );

        if (!root.contains("components") || !root.at("components").is_array()) {
            errors->push_back("manifest.components must be an array");
        } else {
            for (std::size_t index = 0; index < root.at("components").size();
                 ++index) {
                const json& entry = root.at("components").at(index);
                if (!entry.is_object()) {
                    errors->push_back(
                        "manifest.components[" + std::to_string(index)
                        + "] must be a JSON object"
                    );
                    continue;
                }
                value.components.push_back(parse_component(
                    entry, "manifest.components[" + std::to_string(index) + "]",
                    errors
                ));
            }
        }

        if (root.contains("relations")) {
            if (!root.at("relations").is_array()) {
                errors->push_back("manifest.relations must be an array");
            } else {
                value.relations = root.at("relations");
            }
        }

        return value;
    }

    bool is_facade_entry_kind(const std::string& kind) {
        return kind == "static_lib" || kind == "shared_lib"
            || kind == "interface_lib" || kind == "exe" || kind == "qt_app";
    }

    bool contains_key_conflict(
        const std::vector<std::string>& values, const std::string& candidate
    ) {
        const std::string prefix = candidate + "/";
        for (const std::string& value : values) {
            if (value == candidate) {
                return true;
            }
            if (value.rfind(prefix, 0U) == 0U) {
                return true;
            }
            const std::string other_prefix = value + "/";
            if (candidate.rfind(other_prefix, 0U) == 0U) {
                return true;
            }
        }
        return false;
    }

    void validate_external_project(
        const component& component_value, string_list* errors
    ) {
        if (!component_value.stack.contains("external_project")) {
            return;
        }

        const std::string context
            = "component.stack.external_project for " + component_value.id;
        const json& external = component_value.stack.at("external_project");
        if (!external.is_object()) {
            errors->push_back(context + " must be a JSON object");
            return;
        }

        for (const std::string field :
             { "repository", "revision", "component" }) {
            if (!external.contains(field) || !external.at(field).is_string()
                || trim_copy(external.at(field).get<std::string>()).empty()) {
                errors->push_back(
                    context + "." + field + " must be a non-empty string"
                );
            }
        }

        if (external.contains("component")
            && external.at("component").is_string()
            && !is_valid_id(external.at("component").get<std::string>())) {
            errors->push_back(
                context + ".component must match ^[a-z][a-z0-9_]*$"
            );
        }

        const fs::path root(component_value.root);
        const std::string normalized = root.lexically_normal().generic_string();
        if (root.is_absolute() || normalized == ".."
            || normalized.rfind("../", 0U) == 0U) {
            errors->push_back(
                "external component.root must be repository-relative: "
                + component_value.id
            );
        }
        if (!component_value.modules.empty()
            || !component_value.file_units.empty()) {
            errors->push_back(
                "external components must not redeclare repository-owned "
                "modules or file_units: "
                + component_value.id
            );
        }
        for (const artifact& artifact_value : component_value.artifacts) {
            if (artifact_value.kind != "static_lib") {
                errors->push_back(
                    "external_project currently supports static_lib "
                    "artifacts only: "
                    + component_value.id + ":" + artifact_value.id
                );
            }
            if (trim_copy(artifact_value.name).empty()) {
                errors->push_back(
                    "external_project artifacts require their installed "
                    "output name: "
                    + component_value.id + ":" + artifact_value.id
                );
            }
        }
    }

} // namespace manifest_support

using namespace manifest_support;

manifest_report load_manifest(const fs::path& manifest_path) {
    manifest_report report;
    report.path = manifest_path;
    std::error_code status_error;
    report.has_manifest
        = std::filesystem::symlink_status(manifest_path, status_error).type()
        != std::filesystem::file_type::not_found;
    std::ifstream file(manifest_path);
    if (!file.is_open()) {
        report.errors.push_back("missing manifest: " + manifest_path.string());
        return report;
    }

    try {
        json root = json::parse(file);
        if (!root.is_object()) {
            report.errors.push_back("manifest top-level must be a JSON object");
            return report;
        }
        report.has_manifest = true;
        report.value = parse_manifest(root, &report.errors);
        if (report.value.has_value()) {
            const string_list validation_errors
                = validate_manifest(*report.value);
            report.errors.insert(
                report.errors.end(), validation_errors.begin(),
                validation_errors.end()
            );
        }
    } catch (const json::exception& error) {
        report.errors.push_back(
            "invalid JSON in manifest: " + std::string(error.what())
        );
    }

    return report;
}

bool save_manifest(
    const fs::path& manifest_path, const manifest& value,
    std::string* error_message
) {
    std::error_code error;
    fs::create_directories(manifest_path.parent_path(), error);
    if (error) {
        *error_message = "unable to create "
            + manifest_path.parent_path().string() + ": " + error.message();
        return false;
    }

    std::ofstream file(manifest_path, std::ios::binary | std::ios::trunc);
    if (!file.is_open()) {
        *error_message = "unable to open " + manifest_path.string();
        return false;
    }

    file << to_json(value).dump(2) << "\n";
    return true;
}

string_list validate_manifest(const manifest& value) {
    string_list errors;
    if (!is_valid_id(value.id)) {
        errors.push_back("manifest.id must match ^[a-z][a-z0-9_]*$");
    }
    if (trim_copy(value.description).empty()) {
        errors.push_back("manifest.description must not be empty");
    }
    if (!value.version.empty()
        && !std::regex_match(value.version, project_version_pattern)) {
        errors.push_back(
            "manifest.version must be a three-part semantic version"
        );
    }
    if (value.cpp_standard < 17) {
        errors.push_back("manifest.cpp_standard must be at least 17");
    }
    if (!value.android_application_id.empty()
        && !std::regex_match(
            value.android_application_id, android_application_id_pattern
        )) {
        errors.push_back(
            "manifest.android_application_id must be a reverse-DNS identifier"
        );
    }
    if (!value.android_package_source_dir.empty()) {
        const fs::path package_source(value.android_package_source_dir);
        const std::string normalized
            = package_source.lexically_normal().generic_string();
        if (package_source.is_absolute() || normalized == "."
            || normalized == ".." || normalized.rfind("../", 0U) == 0U
            || normalized != value.android_package_source_dir
            || !std::regex_match(
                value.android_package_source_dir,
                android_package_source_dir_pattern
            )) {
            errors.push_back(
                "manifest.android_package_source_dir must be a safe, "
                "non-root project-relative directory"
            );
        }
    }
    if (value.components.empty()) {
        errors.push_back("manifest.components must not be empty");
    }

    std::set<std::string> component_ids;
    std::set<std::string> artifact_refs;
    for (const component& component_value : value.components) {
        if (!is_valid_id(component_value.id)) {
            errors.push_back(
                "component.id must match ^[a-z][a-z0-9_]*$: "
                + component_value.id
            );
        }
        if (!component_ids.insert(component_value.id).second) {
            errors.push_back("duplicate component id: " + component_value.id);
        }
        if (trim_copy(component_value.description).empty()) {
            errors.push_back(
                "component.description must not be empty: " + component_value.id
            );
        }
        if (trim_copy(component_value.root).empty()) {
            errors.push_back(
                "component.root must not be empty: " + component_value.id
            );
        }
        validate_external_project(component_value, &errors);

        std::vector<std::string> seen_modules;
        for (const std::string& module_path : component_value.modules) {
            if (module_path.empty() || module_path == ".") {
                errors.push_back(
                    "component module path must not be empty: "
                    + component_value.id
                );
                continue;
            }
            if (contains_key_conflict(seen_modules, module_path)) {
                errors.push_back(
                    "component modules must not overlap as both leaf and group "
                    "paths: "
                    + component_value.id + ":" + module_path
                );
                continue;
            }
            seen_modules.push_back(module_path);
        }

        std::set<std::string> artifact_ids;
        for (const artifact& artifact_value : component_value.artifacts) {
            if (!is_valid_id(artifact_value.id)) {
                errors.push_back(
                    "artifact.id must match ^[a-z][a-z0-9_]*$: "
                    + component_value.id + ":" + artifact_value.id
                );
            }
            if (!artifact_ids.insert(artifact_value.id).second) {
                errors.push_back(
                    "duplicate artifact id in component " + component_value.id
                    + ": " + artifact_value.id
                );
            }
            if (!artifact_kinds.contains(artifact_value.kind)) {
                errors.push_back(
                    "unsupported artifact kind for " + component_value.id + ":"
                    + artifact_value.id + ": " + artifact_value.kind
                );
            }
            artifact_refs.insert(component_value.id + ":" + artifact_value.id);
        }
        if (component_value.artifacts.empty()) {
            errors.push_back(
                "component.artifacts must not be empty: " + component_value.id
            );
        }

        std::set<std::string> file_unit_ids;
        for (const file_unit& file_unit_value : component_value.file_units) {
            if (trim_copy(file_unit_value.id).empty()) {
                errors.push_back(
                    "file_unit.id must not be empty in component "
                    + component_value.id
                );
            }
            if (!file_unit_ids.insert(file_unit_value.id).second) {
                errors.push_back(
                    "duplicate file_unit id in component " + component_value.id
                    + ": " + file_unit_value.id
                );
            }
            if (!file_unit_kinds.contains(file_unit_value.kind)) {
                errors.push_back(
                    "unsupported file_unit kind for " + component_value.id + ":"
                    + file_unit_value.id + ": " + file_unit_value.kind
                );
            }
        }
    }

    const std::optional<artifact_ref> facade_ref
        = parse_artifact_ref(value.facade_entry_artifact);
    if (!facade_ref.has_value()) {
        errors.push_back(
            "manifest.facade.entry_artifact must be in component:artifact form"
        );
        return errors;
    }

    bool found_facade = false;
    for (const component& component_value : value.components) {
        if (component_value.id != facade_ref->component_id) {
            continue;
        }
        for (const artifact& artifact_value : component_value.artifacts) {
            if (artifact_value.id != facade_ref->artifact_id) {
                continue;
            }
            found_facade = true;
            if (!is_facade_entry_kind(artifact_value.kind)) {
                errors.push_back(
                    "manifest.facade.entry_artifact must reference a "
                    "facade-eligible artifact: "
                    + value.facade_entry_artifact
                );
            }
            break;
        }
    }
    if (!found_facade) {
        errors.push_back(
            "manifest.facade.entry_artifact does not resolve: "
            + value.facade_entry_artifact
        );
    }

    return errors;
}

json to_json(const manifest& value) {
    json root = json::object();
    root["id"] = value.id;
    root["description"] = value.description;
    if (!value.version.empty()) {
        root["version"] = value.version;
    }
    root["cpp_standard"] = value.cpp_standard;
    if (!value.android_application_id.empty()) {
        root["android_application_id"] = value.android_application_id;
    }
    if (!value.android_package_source_dir.empty()) {
        root["android_package_source_dir"] = value.android_package_source_dir;
    }
    if (value.install_assets) {
        root["install_assets"] = true;
    }

    json facade = json::object();
    facade["entry_artifact"] = value.facade_entry_artifact;
    root["facade"] = facade;

    json components_json = json::array();
    for (const component& component_value : value.components) {
        json component_json = json::object();
        component_json["id"] = component_value.id;
        component_json["description"] = component_value.description;
        component_json["root"] = component_value.root;
        if (!component_value.stack.empty()) {
            component_json["stack"] = component_value.stack;
        }
        component_json["modules"] = modules_to_json(component_value.modules);

        if (!component_value.file_units.empty()) {
            json file_units_json = json::array();
            for (const file_unit& file_unit_value :
                 component_value.file_units) {
                json file_unit_json = json::object();
                file_unit_json["id"] = file_unit_value.id;
                file_unit_json["kind"] = file_unit_value.kind;
                file_units_json.push_back(file_unit_json);
            }
            component_json["file_units"] = file_units_json;
        }

        json artifacts_json = json::array();
        for (const artifact& artifact_value : component_value.artifacts) {
            json artifact_json = json::object();
            artifact_json["id"] = artifact_value.id;
            artifact_json["kind"] = artifact_value.kind;
            if (!artifact_value.name.empty()) {
                artifact_json["name"] = artifact_value.name;
            }
            if (!artifact_value.link.empty()) {
                artifact_json["link"] = artifact_value.link;
            }
            artifacts_json.push_back(artifact_json);
        }
        component_json["artifacts"] = artifacts_json;

        if (!component_value.tests.empty()) {
            component_json["tests"] = component_value.tests;
        }
        if (!component_value.benchmarks.empty()) {
            component_json["benchmarks"] = component_value.benchmarks;
        }

        components_json.push_back(component_json);
    }
    root["components"] = components_json;
    if (!value.relations.empty()) {
        root["relations"] = value.relations;
    }
    return root;
}

std::optional<artifact_ref> parse_artifact_ref(const std::string& value) {
    const std::size_t separator = value.find(':');
    if (separator == std::string::npos || separator == 0U
        || separator + 1U >= value.size()) {
        return std::nullopt;
    }
    if (value.find(':', separator + 1U) != std::string::npos) {
        return std::nullopt;
    }
    return artifact_ref {
        value.substr(0U, separator),
        value.substr(separator + 1U),
    };
}

std::string format_artifact_ref(const artifact_ref& value) {
    return value.component_id + ":" + value.artifact_id;
}

} // namespace ecosystem
