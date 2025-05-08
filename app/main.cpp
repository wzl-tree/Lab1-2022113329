#include <atomic>
#include <csignal>
#include <thread>
#include <utility>
#ifdef ENABLE_DOCTEST_IN_LIBRARY
#    define DOCTEST_CONFIG_IMPLEMENT
#    include "doctest/doctest.h"
#endif

#include <algorithm>
#include <cctype>
#include <charconv>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string>
#include <system_error>

#include <fmt/base.h>
#include <fmt/chrono.h>
#include <fmt/ostream.h>

#include "rand_gen.hpp"
#include "word_graph.hpp"

extern void parse_and_populate_graph(std::istream& is, word_graph& graph);

namespace
{
std::atomic<bool> sigint_flag = false;

void signal_handler(int) {
    sigint_flag.store(true, std::memory_order_relaxed);
}

void consume_input() {
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.clear();  // clear the eofbit
}

std::string str_tolower(const std::string& str) {
    std::string result {};
    for (auto ch : str) {
        if (std::isalpha(ch)) {
            result.push_back(static_cast<char>(std::tolower(ch)));
        }
    }
    return result;
};

template<typename Callable>
void profile_function(Callable&& func) {
    auto start_time = std::chrono::high_resolution_clock::now();

    // Call the function object (lambda)
    // Use std::forward to preserve the original value category of the callable
    std::forward<Callable>(func)();

    auto end_time = std::chrono::high_resolution_clock::now();

    // Calculate and return the duration in microseconds
    fmt::println(
        "time elapsed: {}",
        std::chrono::duration_cast<std::chrono::duration<double, std::micro>>(
            end_time - start_time));
}

bool input_path_and_parse(word_graph& graph) {
    std::string path_string;
    std::ifstream input_file;
    while (true) {
        fmt::print("Enter input path, empty line to exit.\n" "Input path: ");
        std::getline(std::cin, path_string);

        if (path_string.empty()) {
            fmt::println("Canceled.");
            return false;
        }

        input_file.open(path_string);
        if (!input_file.is_open()) {
            fmt::println("{} does not exist. Re-enter please.", path_string);
        } else {
            break;
        }
    }

    if (!graph.empty()) {
        graph.clear();
    }

    parse_and_populate_graph(input_file, graph);
    fmt::println("The graph has been parsed.");
    fmt::println("Vertex count: {}. Edges count: {}.",
                 graph.get_num_vertices(),
                 graph.get_num_edges());

    return true;
}

void print_graph(const word_graph& graph) {
    graph.print_graph();
    fmt::print("Generate word_graph.dot file? [y/N] ");
    std::string input {};
    std::getline(std::cin, input);
    if (input != "y" && input != "Y") {
        fmt::println("Canceled: {}.", input);
        return;
    }
    std::ofstream os("word_graph.dot");
    graph.output_graphviz(os);
    os.close();
    fmt::print("Call graphviz to generate word_graph.svg and open it? [y/N] ");
    std::getline(std::cin, input);
    if (input != "y" && input != "Y") {
        fmt::println("Canceled.");
        return;
    }

    std::system("dot -Tpng word_graph.dot -o word_graph.png");
#ifdef _WIN32
    fmt::println("Running on Windows, use start.");
    std::system("start word_graph.png");
#elif defined(__APPLE__)
    fmt::println("Running on MacOS, use open.");
    std::system("open word_graph.png &");
#elif defined(__linux__)
    fmt::println("Running on Linux, use xdg-open.");
    std::system("xdg-open word_graph.png &");
#else
    std::cout << "Unknown platform" << std::endl;
#endif
}

void query_bridge_word(const word_graph& graph) {
    fmt::print("Enter two words.\n" "Input: ");
    std::string from {}, to {};
    std::cin >> from >> to;
    consume_input();

    auto bridge_words =
        graph.get_bridge_words(str_tolower(from), str_tolower(to));

    if (!bridge_words.has_value()) {
        fmt::println("No word1 or word2 in the graph!");
        return;
    }

    if (bridge_words->empty()) {
        fmt::println("No bridge words from word1 to word2!");
        return;
    }

    fmt::print("The bridge words from word1 to word2 are: ");
    if (bridge_words->size() == 1) {
        fmt::println("{}.", bridge_words->front());
    } else {
        std::ranges::for_each(bridge_words->begin(),
                              bridge_words->end() - 1,
                              [](auto e) { fmt::print("{} ", e); });
        fmt::println("and {}.", bridge_words->back());
    }
}

void generate_new_sentense(const word_graph& graph) {
    auto& rng = rand_gen::get_instance();
    fmt::print("Enter a sentense.\n" "Input: ");
    std::string input;
    std::getline(std::cin, input);

    // Handle empty input early
    if (input.empty()) {
        fmt::print("\n");  // Just print a newline for empty input
        return;
    }

    auto tokens_view = input | std::views::split(' ')
        | std::views::filter([](auto&& subrange) { return !subrange.empty(); })
        | std::views::transform([](auto&& subrange)
                                { return std::string_view(subrange); });

    std::string prev_word, cur_word;

    cur_word = tokens_view.front();

    for (auto w : tokens_view | std::views::drop(1)) {
        // the filter_view *caches* the begin iterator, so no re-calculation
        // involved.
        prev_word = w;
        std::swap(prev_word, cur_word);
        auto bridge_words = graph.get_bridge_words(str_tolower(prev_word),
                                                   str_tolower(cur_word));
        if (!bridge_words.has_value() || bridge_words->empty()) {
            fmt::print("{} ", prev_word);
        } else {
            auto rand_bridge_word = (*bridge_words)[rng.generate_random_integer(
                0, static_cast<int>(bridge_words->size()) - 1)];
            fmt::print("{} {} ", prev_word, rand_bridge_word);
        }
    }
    fmt::print("{}\n", cur_word);
}

void find_shortest_path(const word_graph& graph) {
    fmt::print("Enter one or two words.\n" "Input: ");
    std::string input;
    std::getline(std::cin, input);

    std::string_view input_view {input};
    std::string_view src {}, dst {};
    auto first = input_view.find_first_not_of(' ', 0);
    if (first == std::string_view::npos) {
        fmt::println("Canceled.");
    }
    auto last = input_view.find_first_of(' ', first);
    if (last == std::string_view::npos) {
        src = input_view.substr(first);
    } else {
        src = input_view.substr(first, last - first);
        first = input_view.find_first_not_of(' ', last);
        last = input_view.find_first_of(' ', first);
        dst = input_view.substr(
            first, last == std::string_view::npos ? last : last - first);
    }
    profile_function([&] {
      auto result = graph.find_shortest_path(src, dst);
      if (!result) {
        fmt::println("word1 or word2 does not exist.");
      }
    });
}

void page_rank(const word_graph& graph) {
    fmt::println("PageRank Result:");
    profile_function([&] { graph.page_rank(); });
}

void random_walk(const word_graph& graph) {
    sigint_flag.store(false);
    if (std::signal(SIGINT, signal_handler) == SIG_ERR) {
        std::cerr << "Error setting interrupt handler.\n";
        return;
    }

    std::ofstream file("random_walk.txt");
    if (!file.is_open()) {
        std::cerr << "Error opening file.\n";
        return;
    }

    auto iter = graph.random_walk();
    auto first = iter.get();

    if (!first.has_value()) {
        fmt::println("Graph has no vertices.");
        return;
    }

    fmt::println("Random walk, press Ctrl+C to stop:");

    fmt::print("{}", first.value());
    fmt::print(file, "{}", first.value());
    std::cout.flush();

    while (!sigint_flag.load(std::memory_order_relaxed)) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        auto next = iter.next();
        if (next.has_value()) {
            fmt::print(" {}", next.value());
            fmt::print(file, " {}", next.value());
            std::cout.flush();
        } else {
            fmt::println("\nRandom walk finished.");
            break;
        }
    }

    if (sigint_flag.load(std::memory_order_relaxed)) {
        fmt::println("\nRandom walk stopped.");
    }

    if (std::signal(SIGINT, SIG_DFL) == SIG_ERR) {
        std::cerr << "Error restoring interrupt handler.\n";
        return;
    }
}
}  // namespace

/*
void calculate_shortest_path(const word_graph& graph);
void print_graph(const word_graph& graph);
*/

int main() {
    word_graph graph;
    bool is_graph_parsed = false;

    while (true) {
        fmt::print("+------------------------------------+\n"
                   "|     Text to Graph Application      |\n"
                   "|     2203101, 2022113329, wzl       |\n"
                   "+------------------------------------+\n"
                   "|  1. Parse text file from input.    |\n"
                   "|  2. Print constructed graph.       |\n"
                   "|  3. Query bridge words.            |\n"
                   "|  4. Generate new sentense.         |\n"
                   "|  5. Calculate shortest path.       |\n"
                   "|  6. Run PageRank algorithm.        |\n"
                   "|  7. Perform random walk.           |\n"
                   "+------------------------------------+\n\n"
                   "Please input operation, Q/q or Ctrl+D to quit.\n"
                   "default option: 1.\n"
                   "Input: ");

        std::string input {};
        std::getline(std::cin, input);

        if (std::cin.eof()) {
            fmt::println("");
            return EXIT_SUCCESS;
        }

        int user_option = 0;

        if (input == "q") {
            return EXIT_SUCCESS;
        }

        if (input == "") {
            user_option = 1;
        } else {
            auto [_, ec] = std::from_chars(
                input.data(), input.data() + input.size(), user_option);

            if (ec != std::errc()) {
                fmt::println("Unknown input.");
                continue;
            }
        }

        if (user_option == 1) {
            input_path_and_parse(graph);
            is_graph_parsed = true;
            fmt::println("\n\n\n");
            continue;
        }

        if (!is_graph_parsed) {
            fmt::println(
                "You must parse the file first!\n" "Hint: use (1) to parse a file.");
            continue;
        }

        switch (user_option) {
            case 2:
                print_graph(graph);
                break;
            case 3:
                query_bridge_word(graph);
                break;
            case 4:
                generate_new_sentense(graph);
                break;
            case 5:
                find_shortest_path(graph);
                break;
            case 6:
                page_rank(graph);
                break;
            case 7:
                random_walk(graph);
                break;
            default:
                fmt::println("Unknown option. Please input again.");
                break;
        }
        fmt::println("Enter to continue.");
        consume_input();
        fmt::println("\n\n\n");
    }

    return EXIT_SUCCESS;
}
