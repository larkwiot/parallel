#include "include.hpp"

template <char delim>
std::string join(std::vector<std::string>& vec) {
    std::string s = *(vec.begin());
    auto e = vec.end();
    for (auto a = vec.begin() + 1; a != e; a++) {
        s += delim + *a;
    }
    return s;
}

ssize_t find(std::vector<std::string>& vec, const char* value) {
    for (auto i = 0; i < vec.size(); i++) {
        if (vec.at(i) == value) {
            return i;
        }
    }

    return -1;
}

std::vector<std::string> read_file_lines(std::string fn) {
    std::ifstream fh {fn};

    if (!fh) {
        spdlog::error("could not open {}", fn);
        exit(0);
    }

    auto lines = std::vector<std::string> {};

    std::string l;
    while (std::getline(fh, l)) {
        lines.emplace_back(l);
    }
    
    return lines;
}

std::vector<std::string> read_stdin_lines() {
    auto lines = std::vector<std::string> {};

    std::string l;
    while (std::getline(std::cin, l)) {
        lines.emplace_back(l);
    }

    return lines;
}

auto get_cli_args(int argc, char* argv[]) {
    argparse::ArgumentParser p("parallel", "Parallel version: 0.2");

    p.add_argument("command")
        .remaining();
    
    p.add_argument("-d", "--debug")
        .help("set log level to debug")
        .default_value(false)
        .implicit_value(true);

    p.add_argument("--verbose")
        .help("set log level to info")
        .default_value(false)
        .implicit_value(true);
    
    p.add_argument("-t", "--threads")
        .help("manually specify number of threads to use (default is to autodetect)")
        .nargs(1);
    
    p.add_argument("-f", "--file")
        .help("set the file to read inputs for parallel command job (default is stdin)")
        .nargs(1);
    
    try {
        p.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        spdlog::error("Parallel: {}\n{}", err.what(), p.help().str());
        exit(0);
    }

    if (p["--verbose"] == true) {
        if (p["--debug"] == true) {
            spdlog::error("you cannot specify both verbose and debug output levels");
            exit(0);
        }

        spdlog::set_level(spdlog::level::info);

    } else if (p["--debug"] == true) {
        spdlog::set_level(spdlog::level::debug);

    } else {
        spdlog::set_level(spdlog::level::warn);
    }

    // now we start actually setting up an arguments
    // map to pass back to main
    std::unordered_map<std::string, std::string> args = {};

    std::string threads;

    try {
        threads = p.get<std::string>("--threads");
        spdlog::debug("manually set threads to {}", threads);

        if (std::stoi(threads) < 0) {
            spdlog::error("invalid number of threads specified: {} need 1 or more", threads);
            exit(0);
        }

    } catch (std::logic_error& err) {
        spdlog::debug("threads not specified, auto-detecting");
        auto proc_count = std::thread::hardware_concurrency();

        if (proc_count == 0) {
            spdlog::error("unable to auto-detect processor count (threads)");
            exit(0);
        }

        threads = std::to_string(proc_count);
    }

    spdlog::debug("selected {} threads", threads);
    args.emplace("threads", threads);
    
    try {
        auto filename = p.get<std::string>("--file");
        spdlog::debug("will read inputs from file {}", filename);
        args.emplace("filename", filename);
        args.emplace("stdin", "false");
    } catch (std::logic_error& err) {
        spdlog::debug("no file specified, will use stdin");
        args.emplace("filename", "");
        args.emplace("stdin", "true");
    }

    std::string scmd;

    try {
        auto vcmd = p.get<std::vector<std::string>>("command");

        auto bracket_pos = find(vcmd, "{}");

        if (bracket_pos < 0) {
            spdlog::error("could not find \"{}\" in command to place inputs into");
            exit(0);
        }

        scmd = join<' '>(vcmd);
        spdlog::debug("command: {}", scmd);
    } catch (std::logic_error& err) {
        std::cerr << fmt::format("{}\n{}", err.what(), p.help().str());
        exit(0);
    }

    args.emplace("command", scmd);

    return args;
}
