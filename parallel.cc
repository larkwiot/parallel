#include "util.hpp"

void execute(std::string& command_template, std::string& input) {
    auto filled_command = fmt::format(command_template, input);
    spdlog::debug("will execute command: {}", filled_command);
    system(filled_command.c_str());
}

int main(int argc, char* argv[]) {
    // this will get set to the proper value
    // when the arguments are parsed
    spdlog::set_level(spdlog::level::info);
    spdlog::set_pattern("[parallel] [%^%l%$] %v");

    auto args = get_cli_args(argc, argv);

    std::string command_template = args.at("command");
    std::string num_threads = args.at("threads");

    tf::Executor executor(std::stoi(num_threads));
    spdlog::info("created executor with {} threads", num_threads);
    tf::Taskflow taskflow;

    std::vector<std::string> input;

    // yes it's intentional to compare to a string and not
    // a bool. Counter-intuitive but that's what the map
    // stores, not bools
    if (args.at("stdin") == "true") {
        input = read_stdin_lines();
    } else {
        input = read_file_lines(args.at("filename"));
    }
    spdlog::info("got input with {} lines", input.size());

    auto execute_template = [&command_template] (std::string& inp) {
        execute(command_template, inp);
    };

    taskflow.for_each(input.begin(), input.end(), execute_template);
    executor.run(taskflow).wait();

    return 0;
}
