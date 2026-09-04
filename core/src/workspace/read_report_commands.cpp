#include "workspace/read_commands.hpp"

#include "command_internal.hpp"

#include <optional>
#include <string>

namespace ecosystem::command_support {

struct workspace_cxx_report_result {
    json report = json::object();
    command_error status = command_error::ok;
};

workspace_cxx_report_result build_workspace_cxx_report(
    const workspace_context& workspace, const workspace_scope& scope,
    const bool treat_diagnostics_as_failure, std::ostream& err
) {
    workspace_cxx_report_result result;
    result.report["workspace_root"] = workspace.root.string();

    json projects = json::array();
    json errors = json::array();

    for (const workspace_project* project :
         selected_workspace_projects(workspace, scope)) {
        const std::vector<std::optional<artifact_ref>> artifact_requests
            = workspace_artifact_requests_for_project(scope, project);
        for (const std::optional<artifact_ref>& requested_artifact :
             artifact_requests) {
            const std::optional<resolved_artifact> resolved
                = resolve_artifact(project->manifest_value, requested_artifact);
            const bool include_benchmarks = resolved.has_value()
                && component_is_benchmark_only(*resolved->component_value);
            const command_error configure_status = run_configure_build_tree(
                project->root, project->manifest_value, "debug",
                has_tests_enabled(project->manifest_value), false,
                include_benchmarks
                    || has_benchmarks_enabled(project->manifest_value),
                err
            );
            if (configure_status != command_error::ok) {
                result.status = combine_status(result.status, configure_status);
                errors.push_back(
                    project->manifest_value.id
                    + ": configure failed for C++ analysis"
                );
                continue;
            }

            const std::optional<std::string> component_filter
                = requested_artifact.has_value()
                ? std::make_optional(requested_artifact->component_id)
                : std::nullopt;
            const cxx_analysis_report project_report = analyze_project_sources(
                project->manifest_value,
                project->root,
                component_filter,
                true,
                include_benchmarks
            );
            json project_json = to_json(project_report);
            project_json["project"] = project->manifest_value.id;
            project_json["root"]
                = relative_workspace_path(workspace, project->root);
            if (requested_artifact.has_value()) {
                project_json["artifact"] = format_artifact_ref(*requested_artifact);
            }
            projects.push_back(project_json);

            if (!project_report.errors.empty()) {
                for (const std::string& message : project_report.errors) {
                    errors.push_back(project->manifest_value.id + ": " + message);
                }
                result.status = combine_status(
                    result.status, command_error::task_failed
                );
            }
            if (treat_diagnostics_as_failure
                && (project_report.total_errors > 0
                    || project_report.total_warnings > 0)) {
                result.status = combine_status(
                    result.status, command_error::task_failed
                );
            }
        }
    }

    result.report["projects"] = projects;
    result.report["errors"] = errors;
    const json selection = workspace_scope_json(workspace, scope);
    if (!selection.is_null() && !selection.empty()) {
        result.report["selection"] = selection;
    }
    return result;
}

command_error run_report(
    const fs::path& project_root, const manifest& manifest_value,
    const std::string& kind,
    const std::optional<artifact_ref>& requested_artifact,
    std::ostream& out, std::ostream& err
) {
    if (!contains_string(known_report_kinds, kind)) {
        print_error(
            err, command_error::invalid_request,
            "unknown report kind: " + kind
        );
        return command_error::invalid_request;
    }

    ensure_local_artifacts(project_root, false, false, false);

    if (kind == "toolchains") {
        const json report = toolchains_report();
        const fs::path report_path
            = local_report_dir(project_root) / "toolchains.json";
        std::string error_message;
        if (!write_text_file(
                report_path, report.dump(2) + "\n", &error_message
            )) {
            print_error(err, command_error::task_failed, error_message);
            return command_error::task_failed;
        }
        out << report.dump(2) << "\n";
        return command_error::ok;
    }

    if (kind == "matrix") {
        const json report = matrix_report(manifest_value, requested_artifact);
        const fs::path report_path = local_report_dir(project_root) / "matrix.json";
        std::string error_message;
        if (!write_text_file(
                report_path, report.dump(2) + "\n", &error_message
            )) {
            print_error(err, command_error::task_failed, error_message);
            return command_error::task_failed;
        }
        out << report.dump(2) << "\n";
        return command_error::ok;
    }

    command_error status = command_error::ok;
    const std::optional<resolved_artifact> resolved
        = resolve_artifact(manifest_value, requested_artifact);
    const bool include_benchmarks = resolved.has_value()
        && component_is_benchmark_only(*resolved->component_value);
    if (kind == "cxx") {
        status = run_configure_build_tree(
            project_root, manifest_value, "debug",
            has_tests_enabled(manifest_value), false, include_benchmarks, err
        );
        if (status != command_error::ok) {
            return status;
        }
    }

    const std::optional<std::string> component_filter
        = requested_artifact.has_value()
        ? std::make_optional(requested_artifact->component_id)
        : std::nullopt;
    const cxx_analysis_report report = analyze_project_sources(
        manifest_value,
        project_root,
        component_filter,
        true,
        include_benchmarks
    );
    const fs::path report_path = local_report_dir(project_root) / "cxx.json";
    std::string error_message;
    if (!write_text_file(
            report_path, to_json(report).dump(2) + "\n", &error_message
        )) {
        print_error(err, command_error::task_failed, error_message);
        return command_error::task_failed;
    }
    out << to_json(report).dump(2) << "\n";
    return report.errors.empty() ? command_error::ok
                                 : command_error::task_failed;
}

command_error run_workspace_report(
    const workspace_context& workspace, const std::string& kind,
    const workspace_scope& scope, std::ostream& out, std::ostream& err
) {
    if (!contains_string(known_report_kinds, kind)) {
        print_error(
            err, command_error::invalid_request,
            "unknown report kind: " + kind
        );
        return command_error::invalid_request;
    }

    ensure_local_artifacts(workspace.root, false, false, false);

    if (kind == "toolchains") {
        if (workspace_artifact_filter_count(scope) > 0U) {
            print_error(
                err, command_error::invalid_request,
                "toolchains report does not support artifact filters"
            );
            return command_error::invalid_request;
        }
        json report = toolchains_report();
        report["workspace_root"] = workspace.root.string();
        const json selection = workspace_scope_json(workspace, scope);
        if (!selection.is_null() && !selection.empty()) {
            report["selection"] = selection;
        }
        const fs::path report_path
            = local_report_dir(workspace.root) / "workspace_toolchains.json";
        std::string error_message;
        if (!write_text_file(
                report_path, report.dump(2) + "\n", &error_message
            )) {
            print_error(err, command_error::task_failed, error_message);
            return command_error::task_failed;
        }
        out << report.dump(2) << "\n";
        return command_error::ok;
    }

    if (kind == "matrix") {
        const json report = workspace_matrix_report(workspace, scope);
        const fs::path report_path
            = local_report_dir(workspace.root) / "workspace_matrix.json";
        std::string error_message;
        if (!write_text_file(
                report_path, report.dump(2) + "\n", &error_message
            )) {
            print_error(err, command_error::task_failed, error_message);
            return command_error::task_failed;
        }
        out << report.dump(2) << "\n";
        return command_error::ok;
    }

    const workspace_cxx_report_result result
        = build_workspace_cxx_report(workspace, scope, false, err);
    const fs::path report_path
        = local_report_dir(workspace.root) / "workspace_cxx.json";
    std::string error_message;
    if (!write_text_file(
            report_path, result.report.dump(2) + "\n", &error_message
        )) {
        print_error(err, command_error::task_failed, error_message);
        return command_error::task_failed;
    }
    out << result.report.dump(2) << "\n";
    return result.status;
}

}  // namespace ecosystem::command_support

namespace ecosystem {

command_error run_report(
    const std::filesystem::path& project_root, const manifest& manifest_value,
    const std::string& kind,
    const std::optional<artifact_ref>& requested_artifact,
    std::ostream& out, std::ostream& err
) {
    return command_support::run_report(
        project_root, manifest_value, kind, requested_artifact, out, err
    );
}

command_error run_workspace_report(
    const workspace_context& workspace, const std::string& kind,
    const workspace_scope& scope, std::ostream& out, std::ostream& err
) {
    return command_support::run_workspace_report(
        workspace, kind, scope, out, err
    );
}

}  // namespace ecosystem
