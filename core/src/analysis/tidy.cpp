#include "analysis/tidy.hpp"

#include "workspace/project.hpp"
#include "workspace/tooling.hpp"

#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace ecosystem {
namespace tidy_support {

bool path_exists(const fs::path& path) {
    std::error_code error;
    return fs::exists(path, error) && !error;
}

string_list split_nonempty_lines(const std::string& value) {
    string_list lines;
    std::stringstream stream(value);
    std::string line;
    while (std::getline(stream, line)) {
        if (!line.empty()) {
            lines.push_back(line);
        }
    }
    return lines;
}

}  // namespace tidy_support

using namespace tidy_support;

tidy_check_report run_tidy_check(
    const manifest& value,
    const fs::path& project_root,
    const std::optional<std::string>& component_filter,
    const bool include_tests,
    const bool include_benchmarks,
    const std::string& profile
) {
    tidy_check_report report;
    report.analysis = analyze_project_sources(
        value,
        project_root,
        component_filter,
        include_tests,
        include_benchmarks
    );

    const fs::path build_dir = local_build_dir(project_root, profile);
    const fs::path compilation_database_path = build_dir / "compile_commands.json";
    report.compilation_database = compilation_database_path.string();
    report.compilation_database_available = path_exists(compilation_database_path);

    const tool_status clang_tidy = probe_tool("clang-tidy");
    report.clang_tidy_available = clang_tidy.available;
    report.clang_tidy_path = clang_tidy.path;
    if (!report.clang_tidy_available) {
        report.clang_tidy_skip_reason = "clang-tidy is not available";
        return report;
    }
    if (!report.compilation_database_available) {
        report.clang_tidy_skip_reason
            = "compile_commands.json is not available";
        return report;
    }

    const std::vector<cxx_analysis_source> sources = cxx_analysis_sources(
        value,
        project_root,
        component_filter,
        include_tests,
        include_benchmarks
    );
    if (sources.empty()) {
        report.clang_tidy_skip_reason = "no source files available for clang-tidy";
        return report;
    }

    std::vector<std::string> command {
        clang_tidy.path,
        "--quiet",
        "-p",
        build_dir.string(),
        "--warnings-as-errors=*",
    };
    for (const cxx_analysis_source& source : sources) {
        command.push_back(source.path.string());
    }

    const captured_command result = capture_command_result(command, project_root);
    report.clang_tidy_used = true;
    report.clang_tidy_exit_code = result.exit_code;
    report.clang_tidy_output = split_nonempty_lines(result.output);
    return report;
}

json to_json(const tidy_check_report& value) {
    json report = to_json(value.analysis);
    report["compilation_database_available"]
        = value.compilation_database_available;
    report["compilation_database"] = value.compilation_database;
    report["clang_tidy_available"] = value.clang_tidy_available;
    report["clang_tidy_path"] = value.clang_tidy_path;
    report["clang_tidy_used"] = value.clang_tidy_used;
    report["clang_tidy_exit_code"] = value.clang_tidy_exit_code;
    report["clang_tidy_output"] = value.clang_tidy_output;
    report["clang_tidy_skip_reason"] = value.clang_tidy_skip_reason;
    return report;
}

}  // namespace ecosystem
