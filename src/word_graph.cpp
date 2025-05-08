#include <algorithm>
#include <chrono>
#include <csignal>
#include <functional>
#include <iostream>
#include <limits>
#include <optional>
#include <ranges>
#include <set>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "word_graph.hpp"

#include <fmt/base.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>

#include "compact_optional.hpp"
#include "fixed_priority_queue.hpp"
#include "rand_gen.hpp"

class rand_gen;

namespace
{
std::string int_to_identifier(int num) {
    // Characters used for digits (0-9, A-Z)
    static constexpr std::string_view digits(
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
    constexpr int base = digits.size();
    // Handle the case of 0
    if (num == 0) {
        return "a";
    }

    std::string result;

    // Convert the number to the specified base by repeatedly taking remainder
    while (num > 0) {
        int remainder = num % base;
        result +=
            digits[remainder];  // Append the corresponding digit character
        num /= base;  // Divide the number by the base
    }

    // The digits were generated in reverse order, so reverse the result string
    std::ranges::reverse(result);

    return result;
}
}  // namespace

bool word_graph::add_vertex(const std::string& vert) {
    if (!m_bimap.add(vert)) {
        return false;
    }
    m_graph.add_vert();
    return true;
}

bool word_graph::add_vertex(std::string&& vert) {
    if (!m_bimap.add(std::move(vert))) {
        return false;
    }
    m_graph.add_vert();
    return true;
}

// Add an edge to the graph
bool word_graph::add_edge(std::string_view from, std::string_view to) {
    auto index_from = try_get_index(from);
    if (!index_from) {
        return false;
    }
    auto index_to = try_get_index(to);
    if (!index_to) {
        return false;
    }
    m_graph.add_edge(*index_from, *index_to);
    return true;
}

void word_graph::print_graph() const {
    fmt::println("Word Graph:");
    fmt::print("Vertices: ");
    std::ranges::for_each(m_bimap.get_strings(),
                          [](const auto& vert) { fmt::print("{} ", vert); });
    fmt::println("\nEdges:");
    print_edges(std::cout,
                [this](index_type from, index_type to, weight weight)
                {
                    return fmt::format("{} -> {} weight={}\n",
                                       m_bimap.get_string(from),
                                       m_bimap.get_string(to),
                                       weight);
                });
}

void word_graph::output_graphviz(std::ostream& os) const {
    fmt::println(os, "digraph \"word-graph\" {{");
    for (int i = 0; auto label : m_bimap.get_strings()) {
        fmt::println(os, "{} [label=\"{}\"]", int_to_identifier(i), label);
        i++;
    }
    print_edges(os,
                [](index_type from, index_type to, weight weight)
                {
                    return fmt::format("{} -> {} [label=\"{}\"]\n",
                                       int_to_identifier(from),
                                       int_to_identifier(to),
                                       weight);
                });
    fmt::println(os, "}}");
}

void word_graph::print_edges(
    std::ostream& os,
    std::function<std::string(index_type, index_type, weight)> formatter)
    const {
    for (int from_i = 0; from_i < get_num_vertices(); ++from_i) {
        std::ranges::for_each(m_graph.get_successors(from_i),
                              [&](const auto to)
                              { os << formatter(from_i, to.i, to.w); });
    }
}

bool word_graph::find_shortest_path(std::string_view src_str,
                                    std::string_view dst_str) const {
    auto src = try_get_index(src_str),
         dst = dst_str.empty() ? std::nullopt : try_get_index(dst_str);
    if (!src || (!dst_str.empty() && !dst)) {
        return false;
    }

    const auto [distances, previous] = m_graph.dijkstra(*src);

    auto print_path = [&](index_type dst)
    {
        if (distances[dst] == std::numeric_limits<weight>::max()) {
            fmt::println(
                "No path from {} to {}.", src_str, m_bimap.get_string(dst));
            return;
        }
        fmt::println("Shortest path from {} to {}, total weight: {}.",
                     src_str,
                     m_bimap.get_string(dst),
                     distances[dst]);
        std::vector<index_type> path;

        for (index_type i = dst; i != *src; i = previous[i]) {
            path.push_back(previous[i]);
        }

        std::ranges::for_each(std::views::reverse(path),
                              [&](auto v)
                              { fmt::print("{} -> ", m_bimap.get_string(v)); });
        fmt::println("{}", m_bimap.get_string(dst));
    };

    if (dst.has_value()) {
        print_path(*dst);
    } else {
        for (index_type i = 0; i < get_num_vertices(); ++i) {
            if (i != *src) {
                print_path(i);
            }
        }
    }

    return true;
}

std::optional<std::vector<std::string_view>> word_graph::get_bridge_words(
    std::string_view from, std::string_view to) const {
    auto src = try_get_index(from);
    if (!src) {
        return std::nullopt;
    }
    auto dst = try_get_index(to);
    if (!dst) {
        return std::nullopt;
    }
    std::vector<bool> hashmap(get_num_vertices(), false);
    std::ranges::for_each(m_graph.get_successors(*src),
                          [&](edge e) { hashmap[e.i] = true; });
    auto bridge_words_view = m_graph.get_predecessors(*dst)
        | std::views::filter([&](edge e) { return hashmap[e.i]; })
        | std::views::transform([&](edge e)
                                { return m_bimap.get_string(e.i); });
    return std::ranges::to<std::vector<std::string_view>>(bridge_words_view);
}

void word_graph::page_rank() const {
    constexpr auto print_threshold = 300;
    constexpr auto largest_num = 10;

    if (m_graph.get_num_vertices() == 0) {
        fmt::println("No vertices in the graph.");
        return;
    }

    auto n = get_num_vertices();
    auto pr = m_graph.page_rank();
    bool print_verbose = true;
    if (n > print_threshold) {
        fmt::print(
            "Graph has more than {} vertices, still print verbose info? [y/N] ",
            print_threshold);
        std::string input {};
        std::getline(std::cin, input);
        print_verbose = (input == "Y" || input == "y");
    }

    using heap_pair = std::pair<index_type, double>;
    auto cmp_greater = [](const heap_pair& lhs, const heap_pair& rhs)
    { return lhs.second > rhs.second; };
    fixed_priority_queue<heap_pair, decltype(cmp_greater)> min_pq(largest_num,
                                                                  cmp_greater);

    for (index_type i = 0; i < get_num_vertices(); ++i) {
        if (print_verbose) {
            fmt::println("{}: {}", m_bimap.get_string(i), pr[i]);
        }
        min_pq.push({i, pr[i]});
    }
    min_pq.sort();

    fmt::println("The largest {} vertices:", min_pq.size());
    for (const auto& [i, pr] : min_pq.as_span()) {
        fmt::println("{}: {}", m_bimap.get_string(i), pr);
    }
}

std::optional<std::string_view> word_graph::random_walk_iterator::get() {
    if (!finished) {
        return graph.m_bimap.get_string(current);
    }
    return std::nullopt;
}

std::optional<std::string_view> word_graph::random_walk_iterator::next() {
    auto& rg = rand_gen::get_instance();
    if (finished) {
        return std::nullopt;
    }

    const auto& out_edges = graph.m_graph.get_successors(current);

    if (out_edges.empty()) {
        finished = true;
        return std::nullopt;
    }

    const int rand_next =
        rg.generate_random_integer(0, static_cast<int>(out_edges.size()) - 1);
    const index_type next = out_edges[rand_next].i;

    if (visited_edges.contains({current, next})) {
        finished = true;
        return std::nullopt;
    }

    visited_edges.emplace(current, next);
    current = next;

    return graph.m_bimap.get_string(next);
}

word_graph::random_walk_iterator word_graph::random_walk() const {
    return random_walk_iterator(*this);
}

#ifdef ENABLE_DOCTEST_IN_LIBRARY
#    include "doctest/doctest.h"

TEST_CASE("word_graph") {
    SUBCASE("add_vert()") {
        word_graph graph;
        CHECK(graph.add_vertex("1"));
        CHECK(graph.has_vertex("1"));
        CHECK(graph.add_vertex("2"));
        CHECK(graph.has_vertex("2"));
        CHECK(graph.get_num_vertices() == 2);
    }
    SUBCASE("add_edge()") {
        word_graph graph;
        CHECK(graph.get_num_vertices() == 2);
        CHECK(!graph.add_edge("2", "3"));
        CHECK(graph.add_edge("1", "2"));
    }
}
#endif
