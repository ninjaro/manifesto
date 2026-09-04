#include "cli/manifesto_cli.hpp"

#include "cli/common_args.hpp"
#include "cli/engels_cli.hpp"
#include "cli/marx_cli.hpp"

#include <cxxopts.hpp>

#include <algorithm>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <exception>
#include <iostream>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace ecosystem::manifesto_cli_support {

struct queued_request {
    enum class state {
        pending,
        running,
        completed,
        failed,
        skipped,
    };

    std::size_t id = 0U;
    std::string owner = "engels";
    args_list args;
    std::string summary;
    state current_state = state::pending;
    int exit_status = 0;
    std::string output;
    std::string error;
};

struct manifesto_frontdoor_request {
    bool quiet = false;
    bool interactive = false;
    bool help = false;
    args_list request_args;
};

std::string manifesto_usage_text() {
    return "usage: manifesto [--quiet] [<command> ... [--then <command> ...]]\n"
           "\n"
           "batch mode:\n"
           "  manifesto check tests\n"
           "  manifesto sync --project manifesto --then report matrix\n"
           "\n"
           "interactive mode:\n"
           "  manifesto\n"
           "\n"
           "interactive commands:\n"
           "  help\n"
           "  queue\n"
           "  log <id>\n"
           "  skip <id>\n"
           "  clear\n"
           "  exit\n";
}

bool is_manifesto_option_token(const std::string& value) {
    return value == "--quiet" || value == "--interactive"
        || value == "--help" || value == "-h" || value == "help"
        || value == "interactive";
}

std::string normalized_manifesto_option_token(const std::string& value) {
    if (value == "help") {
        return "--help";
    }
    if (value == "interactive") {
        return "--interactive";
    }
    return value;
}

void split_manifesto_option_prefix(
    const args_list& args, args_list* option_args, args_list* request_args
) {
    std::size_t index = 0U;
    for (; index < args.size(); ++index) {
        if (!is_manifesto_option_token(args[index])) {
            break;
        }
        option_args->push_back(normalized_manifesto_option_token(args[index]));
    }
    request_args->assign(args.begin() + static_cast<long>(index), args.end());
}

std::vector<char*> argv_for_args(const args_list& args) {
    static char command_name[] = "manifesto";
    std::vector<char*> argv;
    argv.reserve(args.size() + 1U);
    argv.push_back(command_name);
    for (const std::string& arg : args) {
        argv.push_back(const_cast<char*>(arg.c_str()));
    }
    return argv;
}

std::optional<manifesto_frontdoor_request> parse_manifesto_frontdoor_request(
    const args_list& args, std::string* error_message
) {
    args_list option_args;
    manifesto_frontdoor_request request;
    split_manifesto_option_prefix(args, &option_args, &request.request_args);

    cxxopts::Options options("manifesto", "");
    options.add_options()(
        "quiet", "Suppress successful batch-stage output",
        cxxopts::value<bool>()->default_value("false")->implicit_value("true")
    )(
        "interactive", "Enter the interactive manifesto shell",
        cxxopts::value<bool>()->default_value("false")->implicit_value("true")
    )(
        "help", "Show manifesto usage",
        cxxopts::value<bool>()->default_value("false")->implicit_value("true")
    );

    try {
        std::vector<char*> argv = argv_for_args(option_args);
        const int argc = static_cast<int>(argv.size());
        char** argv_data = argv.data();
        const cxxopts::ParseResult result = options.parse(argc, argv_data);
        request.quiet = result["quiet"].as<bool>();
        request.interactive = result["interactive"].as<bool>();
        request.help = result["help"].as<bool>();
    } catch (const cxxopts::exceptions::exception&) {
        *error_message = manifesto_usage_text();
        return std::nullopt;
    }

    if (request.help && (!request.request_args.empty() || request.interactive)) {
        *error_message = manifesto_usage_text();
        return std::nullopt;
    }
    if (request.interactive && !request.request_args.empty()) {
        *error_message
            = "manifesto interactive mode does not accept queued requests";
        return std::nullopt;
    }
    return request;
}

std::vector<std::string> split_words(const std::string& line) {
    std::istringstream stream(line);
    std::vector<std::string> words;
    std::string value;
    while (stream >> value) {
        words.push_back(value);
    }
    return words;
}

std::string join_words(const std::vector<std::string>& words) {
    std::ostringstream stream;
    for (std::size_t index = 0U; index < words.size(); ++index) {
        if (index != 0U) {
            stream << " ";
        }
        stream << words[index];
    }
    return stream.str();
}

std::string state_label(const queued_request::state value) {
    switch (value) {
    case queued_request::state::pending:
        return "pending";
    case queued_request::state::running:
        return "running";
    case queued_request::state::completed:
        return "completed";
    case queued_request::state::failed:
        return "failed";
    case queued_request::state::skipped:
        return "skipped";
    }
    return "pending";
}

std::optional<std::string> command_owner(const std::string& command_name) {
    if (command_name == "list" || command_name == "check"
        || command_name == "doctor" || command_name == "report") {
        return std::string("engels");
    }
    if (command_name == "sync" || command_name == "mutate"
        || command_name == "build" || command_name == "benchmark"
        || command_name == "run" || command_name == "prerelease") {
        return std::string("marx");
    }
    return std::nullopt;
}

std::optional<std::vector<args_list>> split_manifesto_requests(
    const args_list& args, std::string* error_message
) {
    std::vector<args_list> requests;
    args_list current;
    for (const std::string& arg : args) {
        if (arg != "--then") {
            current.push_back(arg);
            continue;
        }
        if (current.empty()) {
            *error_message = "manifesto request chain cannot start with --then";
            return std::nullopt;
        }
        requests.push_back(current);
        current.clear();
    }

    if (current.empty()) {
        *error_message = "manifesto request chain cannot end with --then";
        return std::nullopt;
    }
    requests.push_back(current);
    return requests;
}

std::string frontend_label(const std::string& owner) {
    return owner;
}

int run_actor_cli(
    const std::string& owner, const args_list& args, std::ostream& out,
    std::ostream& err
) {
    if (owner == "engels") {
        return run_engels_cli(args, out, err);
    }
    if (owner == "marx") {
        return run_marx_cli(args, out, err);
    }
    emit_command_error(
        err, command_error::invalid_request, "unknown manifesto actor"
    );
    return exit_code(command_error::invalid_request);
}

class shell_queue {
public:
    explicit shell_queue(const bool quiet) : quiet_(quiet) {
        worker_ = std::thread(&shell_queue::worker_loop, this);
    }

    ~shell_queue() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stopping_ = true;
            for (const std::size_t id : pending_ids_) {
                requests_[id - 1U].current_state = queued_request::state::skipped;
            }
            pending_ids_.clear();
        }
        condition_.notify_all();
        if (worker_.joinable()) {
            worker_.join();
        }
    }

    void enqueue(
        const std::string& owner, const args_list& args, std::ostream& out
    ) {
        std::size_t request_id = 0U;
        std::string summary;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            request_id = requests_.size() + 1U;
            summary = join_words(args);
            requests_.push_back(
                queued_request {
                    request_id,
                    owner,
                    args,
                    summary,
                    queued_request::state::pending,
                    0,
                    {},
                    {},
                }
            );
            pending_ids_.push_back(request_id);
        }
        condition_.notify_all();
        out << "queued #" << request_id << " -> " << frontend_label(owner)
            << " " << summary << "\n";
    }

    void emit_queue(std::ostream& out) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (requests_.empty()) {
            out << "queue: empty\n";
            return;
        }

        out << "queue:\n";
        for (const queued_request& request : requests_) {
            out << "  #" << request.id << " [" << state_label(request.current_state)
                << "] " << frontend_label(request.owner) << " "
                << request.summary << "\n";
        }
    }

    void emit_log(const std::size_t id, std::ostream& out, std::ostream& err) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (id == 0U || id > requests_.size()) {
            err << "error[invalid_request]: unknown queue item: "
                << std::to_string(id) << "\n";
            return;
        }

        const queued_request& request = requests_[id - 1U];
        out << "#" << request.id << " " << frontend_label(request.owner) << " "
            << request.summary << " [" << state_label(request.current_state)
            << "]\n";
        if (!request.output.empty()) {
            out << request.output;
            if (request.output.back() != '\n') {
                out << "\n";
            }
        }
        if (!request.error.empty()) {
            out << request.error;
            if (request.error.back() != '\n') {
                out << "\n";
            }
        }
    }

    void skip(const std::size_t id, std::ostream& out, std::ostream& err) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (id == 0U || id > requests_.size()) {
            err << "error[invalid_request]: unknown queue item: "
                << std::to_string(id) << "\n";
            return;
        }
        queued_request& request = requests_[id - 1U];
        if (request.current_state != queued_request::state::pending) {
            err << "error[invalid_request]: queue item cannot be skipped: #"
                << request.id << "\n";
            return;
        }
        pending_ids_.erase(
            std::remove(pending_ids_.begin(), pending_ids_.end(), id),
            pending_ids_.end()
        );
        request.current_state = queued_request::state::skipped;
        out << "skipped #" << request.id << "\n";
    }

    void clear(std::ostream& out) {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const std::size_t id : pending_ids_) {
            requests_[id - 1U].current_state = queued_request::state::skipped;
        }
        const std::size_t cleared = pending_ids_.size();
        pending_ids_.clear();
        out << "cleared " << cleared << " pending item(s)\n";
    }

private:
    void worker_loop() {
        while (true) {
            queued_request request;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                condition_.wait(
                    lock, [this]() { return stopping_ || !pending_ids_.empty(); }
                );
                if (stopping_ && pending_ids_.empty()) {
                    return;
                }
                const std::size_t id = pending_ids_.front();
                pending_ids_.pop_front();
                request = requests_[id - 1U];
                requests_[id - 1U].current_state = queued_request::state::running;
            }

            if (!quiet_) {
                std::lock_guard<std::mutex> io_lock(io_mutex_);
                std::cout << "running #" << request.id << " via "
                          << frontend_label(request.owner) << ": "
                          << request.summary << "\n";
            }

            std::ostringstream request_out;
            std::ostringstream request_err;
            const int exit_status = run_actor_cli(
                request.owner, request.args, request_out, request_err
            );

            {
                std::lock_guard<std::mutex> lock(mutex_);
                queued_request& stored = requests_[request.id - 1U];
                stored.exit_status = exit_status;
                stored.output = request_out.str();
                stored.error = request_err.str();
                stored.current_state = exit_status == 0
                    ? queued_request::state::completed
                    : queued_request::state::failed;
            }

            std::lock_guard<std::mutex> io_lock(io_mutex_);
            std::cout << "completed #" << request.id << " via "
                      << frontend_label(request.owner) << " with exit "
                      << exit_status << "\n";
            if (exit_status != 0) {
                if (!request_out.str().empty()) {
                    std::cout << request_out.str();
                    if (request_out.str().back() != '\n') {
                        std::cout << "\n";
                    }
                }
                if (!request_err.str().empty()) {
                    std::cerr << request_err.str();
                    if (request_err.str().back() != '\n') {
                        std::cerr << "\n";
                    }
                }
            }
        }
    }

    bool quiet_ = false;
    bool stopping_ = false;
    std::mutex mutex_;
    std::mutex io_mutex_;
    std::condition_variable condition_;
    std::thread worker_;
    std::deque<std::size_t> pending_ids_;
    std::vector<queued_request> requests_;
};

int dispatch_manifesto_requests(
    const std::vector<args_list>& requests, const bool quiet, std::ostream& out,
    std::ostream& err
) {
    int status = 0;
    for (const args_list& request : requests) {
        if (request.empty()) {
            continue;
        }
        const std::optional<std::string> owner = command_owner(request.front());
        if (!owner.has_value()) {
            emit_command_error(
                err, command_error::invalid_request,
                "unknown command: " + request.front()
            );
            return exit_code(command_error::invalid_request);
        }

        if (quiet) {
            std::ostringstream request_out;
            std::ostringstream request_err;
            const int request_status
                = run_actor_cli(*owner, request, request_out, request_err);
            if (request_status != 0) {
                if (!request_out.str().empty()) {
                    out << request_out.str();
                    if (request_out.str().back() != '\n') {
                        out << "\n";
                    }
                }
                if (!request_err.str().empty()) {
                    err << request_err.str();
                    if (request_err.str().back() != '\n') {
                        err << "\n";
                    }
                }
                return request_status;
            }
            status = request_status;
            continue;
        }

        const int request_status = run_actor_cli(*owner, request, out, err);
        if (request_status != 0) {
            return request_status;
        }
        status = request_status;
    }
    return status;
}

int run_manifesto_shell(std::ostream& out, std::ostream& err) {
    shell_queue queue(false);
    out << "manifesto interactive mode\n";
    out << "type `help` for commands\n";

    std::string line;
    while (true) {
        out << "manifesto> ";
        out.flush();
        if (!std::getline(std::cin, line)) {
            out << "\n";
            return 0;
        }

        const std::vector<std::string> words = split_words(line);
        if (words.empty()) {
            continue;
        }
        if (words.front() == "help") {
            out << manifesto_usage_text();
            continue;
        }
        if (words.front() == "queue") {
            queue.emit_queue(out);
            continue;
        }
        if (words.front() == "clear") {
            queue.clear(out);
            continue;
        }
        if (words.front() == "exit" || words.front() == "quit") {
            return 0;
        }
        if (words.front() == "skip") {
            if (words.size() != 2U) {
                err << "error[invalid_request]: expected: skip <id>\n";
                continue;
            }
            try {
                queue.skip(std::stoul(words[1]), out, err);
            } catch (const std::exception&) {
                err << "error[invalid_request]: invalid queue id: " << words[1]
                    << "\n";
            }
            continue;
        }
        if (words.front() == "log") {
            if (words.size() != 2U) {
                err << "error[invalid_request]: expected: log <id>\n";
                continue;
            }
            try {
                queue.emit_log(std::stoul(words[1]), out, err);
            } catch (const std::exception&) {
                err << "error[invalid_request]: invalid queue id: " << words[1]
                    << "\n";
            }
            continue;
        }
        if (words.front() == "test" || words.front() == "tests") {
            out << "scope (optional): ";
            out.flush();
            std::string scope_line;
            if (!std::getline(std::cin, scope_line)) {
                return 0;
            }
            args_list request { "check", "tests" };
            const std::vector<std::string> scope_words = split_words(scope_line);
            request.insert(request.end(), scope_words.begin(), scope_words.end());
            queue.enqueue("engels", request, out);
            continue;
        }

        std::string request_error;
        const std::optional<std::vector<args_list>> requests
            = split_manifesto_requests(words, &request_error);
        if (!requests.has_value()) {
            emit_command_error(
                err, command_error::invalid_request, request_error
            );
            continue;
        }
        for (const args_list& request : *requests) {
            if (request.empty()) {
                continue;
            }
            const std::optional<std::string> owner = command_owner(request.front());
            if (!owner.has_value()) {
                emit_command_error(
                    err, command_error::invalid_request,
                    "unknown command: " + request.front()
                );
                continue;
            }
            queue.enqueue(*owner, request, out);
        }
    }
}

}  // namespace ecosystem::manifesto_cli_support

namespace ecosystem {

int run_manifesto(const int argc, const char* const* argv) {
    const args_list args = collect_cli_args(argc, argv);
    return run_manifesto_cli(args, std::cout, std::cerr);
}

int run_manifesto_cli(
    const std::vector<std::string>& args, std::ostream& out, std::ostream& err
) {
    std::string parse_error;
    const std::optional<manifesto_cli_support::manifesto_frontdoor_request>
        request = manifesto_cli_support::parse_manifesto_frontdoor_request(
            args, &parse_error
        );
    if (!request.has_value()) {
        emit_command_error(
            err, command_error::invalid_request, parse_error
        );
        return exit_code(command_error::invalid_request);
    }

    if (request->help) {
        out << manifesto_cli_support::manifesto_usage_text();
        return 0;
    }
    if (request->interactive || request->request_args.empty()) {
        return manifesto_cli_support::run_manifesto_shell(out, err);
    }

    std::string chain_error;
    const std::optional<std::vector<args_list>> requests
        = manifesto_cli_support::split_manifesto_requests(
            request->request_args, &chain_error
        );
    if (!requests.has_value()) {
        emit_command_error(
            err, command_error::invalid_request, chain_error
        );
        return exit_code(command_error::invalid_request);
    }

    return manifesto_cli_support::dispatch_manifesto_requests(
        *requests, request->quiet, out, err
    );
}

}  // namespace ecosystem
