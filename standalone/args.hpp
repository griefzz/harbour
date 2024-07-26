///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///

#pragma once

#include <any>
#include <format>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include <ankerl/unordered_dense.h>

/// @brief The Arg type used inside Args
struct Arg {
    std::string name_long;  // long name of the argument
    std::string name_short; // short name of the argument
    std::string description;// description of the argument

    /// @brief Convert an Arg to a std::string
    /// @return converted Arg as std::string
    auto to_string() const -> std::string {
        return std::format("{}\t{}\t{}", name_short, name_long, description);
    }
};

/// @brief A class for handling command line arguments
/// @code
/// auto args = Args::create(targc, targv)
///                        .var("name", "Name of person to use.")
///                        .var("count", "How many times to print.")
///                        .flag("version");
///
/// auto version = args.get<bool>("version");
/// if (version.has_value()) {
///     args.print();
///     return 0;
/// }
///
/// auto count = args.get<size_t>("count");
/// auto name  = args.get<std::string>("name");
/// if (count.has_value() && name.has_value()) {
///     for (auto i = 0; i < *count; i++) {
///         std::cout << "Hello " << *name << "\n";
///     }
/// }
/// @endcode
class Args {
public:
    /// @brief Create a new Args from argc and argv
    /// @param argc Number of arguments
    /// @param argv Arguments
    /// @return Args
    static auto create(int argc, char **argv) -> Args {
        return Args(argc, argv);
    }

    /// @brief Add a var to our list of arguments.
    ///        Vars are arguments that have parameters
    /// @param arg Var argument to add
    /// @return Reference to Args for chaining
    auto var(const Arg &arg) -> Args & {
        arg_list.push_back(arg);

        for (auto i = 0; i < argc; i++) {
            std::string c(argv[i]);
            if ((c == arg.name_long || c == arg.name_short) && (i + 1 < argc)) {
                auto v = std::string(argv[i + 1]);
                if (!arg_map.contains(arg.name_long))
                    arg_map[arg.name_long] = v;
                if (!arg_map.contains(arg.name_short))
                    arg_map[arg.name_short] = v;
            }
        }

        return *this;
    }

    /// @brief Add a var to our list of arguments.
    ///        Vars are arguments that have parameters
    /// @param arg Var name to add
    /// @param desc Description of the var
    /// @return Reference to Args for chaining
    auto var(const std::string &arg, const std::string &desc = "") -> Args & {
        auto v        = str2arg(arg);
        v.description = desc;
        return var(v);
    }

    /// @brief Add a flag to our list of arguments.
    ///        Flags are boolean values that take no extra args.
    /// @param arg Flag argument to add
    /// @return Reference to Args for chaining
    auto flag(const Arg &arg) -> Args & {
        arg_list.push_back(arg);

        for (auto i = 0; i < argc; i++) {
            std::string c(argv[i]);
            if ((c == arg.name_long || c == arg.name_short)) {
                if (!arg_map.contains(arg.name_long))
                    arg_map[arg.name_long] = "1";
                if (!arg_map.contains(arg.name_short))
                    arg_map[arg.name_short] = "1";
            }
        }

        return *this;
    }

    /// @brief Add a flag to our list of arguments.
    ///        Flags are boolean values that take no extra args.
    /// @param arg Flag name to add
    /// @param desc Description of the flag
    /// @return Reference to Args for chaining
    auto flag(const std::string &arg, const std::string &desc = "") -> Args & {
        auto f        = str2arg(arg);
        f.description = desc;
        return flag(f);
    }

    /// @brief Optionally retrieve a value from our arguments
    /// @tparam T Type of value to retrieve (use bool for flags)
    /// @param key Flag or var to retrieve
    /// @return Arguments value or bool for flag
    template<typename T>
    constexpr auto get(const std::string &key) const -> std::optional<T> {
        auto arg = str2arg(key);

        if (auto v = arg_map.find(arg.name_long); v != arg_map.end()) {
            try {
                return cvt<T>(v->second);
            } catch (const std::exception &e) {
                std::cout << e.what() << '\n';
                return {};
            }
        }

        if (auto v = arg_map.find(arg.name_short); v != arg_map.end()) {
            try {
                return cvt<T>(v->second);
            } catch (const std::exception &e) {
                std::cout << e.what() << '\n';
                return {};
            }
        }

        return {};
    }

    /// @brief Print program usage
    auto print() const -> void {
        std::cout << std::format("Usage: {} [options]\n", argv[0]);
        for (const auto &arg: arg_list) {
            std::cout << arg.to_string() << "\n";
        }
    }

private:
    explicit Args(int argc, char **argv) : argc(argc), argv(argv) {}

    template<typename T>
    auto cvt(const std::string &str) const -> T {
        std::istringstream iss(str);
        T value;
        if (!(iss >> value)) {
            throw std::runtime_error("cvt failed to convert str");
        }
        return value;
    }

    constexpr auto str2arg(const std::string &str) const -> Arg {
        return {"--" + str, "-" + std::string(1, str[0]), ""};
    }

    int argc;
    char **argv;

    std::vector<Arg> arg_list;
    ankerl::unordered_dense::map<std::string, std::string> arg_map;
};