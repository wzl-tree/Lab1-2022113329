#include <cctype>
#include <istream>
#include <array>

#include <fmt/base.h>

#include "word_graph.hpp"

namespace
{
bool parse_word(std::istream& is, std::string& str) {
    str.clear();
    char ch = 0;
    while (is.get(ch)) {
        if (std::isalpha(ch)) {
            str.push_back(static_cast<char>(std::tolower(ch)));
        } else {
            is.putback(ch);
            break;
        }
    }
    return !is.eof();
}

bool parse_delims(std::istream& is) {
    bool have_space = false;
    char ch = 0;
    while (is.get(ch)) {
        if (std::isalpha(ch)) {
            is.putback(ch);
            return have_space;
        }
        have_space = true;
    }
    return have_space;
}
}  // namespace

extern void parse_and_populate_graph(std::istream& is, word_graph& graph) {
    std::array<std::string, 2> words {};
    bool current_idx = false;
    parse_delims(is);
    if (!parse_word(is, words[current_idx])) {
        return;
    }
    graph.add_vertex(words[current_idx]);
    bool prev_delim_is_single_space = false;
    while (true) {
        prev_delim_is_single_space = parse_delims(is);
        current_idx = !current_idx;
        if (!parse_word(is, words[current_idx])) {
            return;
        }
        graph.add_vertex(words[current_idx]);
        if (prev_delim_is_single_space) {
            graph.add_edge(words[!current_idx], words[current_idx]);
        }
    }
}
