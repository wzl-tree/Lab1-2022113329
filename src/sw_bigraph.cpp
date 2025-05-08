#include <algorithm>
#include <optional>

#include "sw_bigraph.hpp"

sparse_weighted_bigraph::opt_weight sparse_weighted_bigraph::get_weight(
    index_type from, index_type to) const {
    assert(from >= 0 && from < m_num_vertices && to >= 0
           && to < m_num_vertices);
    const auto& successors = m_adj_list[from];
    auto it = std::ranges::find_if(successors,
                                   [to](const edge& e) { return e.i == to; });
    if (it != successors.end()) {
        return it->w;
    }
    return std::nullopt;
}

bool sparse_weighted_bigraph::add_edge(index_type from, index_type to) {
    assert(from >= 0 && from < m_num_vertices && to >= 0
           && to < m_num_vertices);
    auto successors = get_edges_mut(from, /*successors=*/true);
    auto edge_fwd =
        std::ranges::find_if(successors, [to](edge e) { return e.i == to; });
    if (edge_fwd == successors.end()) {
        m_num_edges += 1;
        m_adj_list[from].emplace_back(to, 1);
        m_rev_adj_list[to].emplace_back(from, 1);
        return false;
    }

    edge_fwd->w += 1;
    auto predecessors = get_edges_mut(to, /*successors=*/false);
    auto edge_inv = std::ranges::find_if(
        predecessors, [from](edge e) { return e.i == from; });
    assert(edge_inv != predecessors.end());

    edge_inv->w += 1;
    return true;
}

#ifdef ENABLE_DOCTEST_IN_LIBRARY
#    include "doctest/doctest.h"

TEST_CASE("sparse_weighted_bigraph") {
    SUBCASE("Constructor and empty()") {
        sparse_weighted_bigraph graph0;
        CHECK(graph0.empty());
        CHECK(graph0.get_num_vertices() == 0);

        sparse_weighted_bigraph graph1(5);
        CHECK(!graph1.empty());
        CHECK(graph1.get_num_vertices() == 5);
    }

    SUBCASE("resize()") {
        sparse_weighted_bigraph graph;
        graph.resize(3);
        CHECK(graph.get_num_vertices() == 3);
        graph.resize(7);
        CHECK(graph.get_num_vertices() == 7);
    }

    SUBCASE(
        "add_edge(), get_weight() and get_successors() / get_predecessors()") {
        sparse_weighted_bigraph graph(5);
        CHECK(graph.get_num_vertices() == 5);

        CHECK(!graph.add_edge(0, 1));
        CHECK(!graph.add_edge(0, 2));
        CHECK(!graph.add_edge(1, 2));
        CHECK(!graph.add_edge(2, 3));
        CHECK(graph.add_edge(2, 3));
        CHECK(graph.add_edge(2, 3));

        // Check successor edges
        const auto& succ_0 = graph.get_successors(0);
        CHECK(succ_0.size() == 2);
        CHECK(graph.get_weight(0, 1) == 1);
        CHECK(graph.get_weight(0, 2) == 1);

        const auto& succ_2 = graph.get_successors(2);
        CHECK(succ_2.size() == 1);
        CHECK(graph.get_weight(2, 3) == 3);

        const auto& succ_4 = graph.get_successors(4);
        CHECK(succ_4.empty());
        CHECK(!graph.get_weight(4, 1));

        // Check predecessor edges
        const auto& pred_2 = graph.get_predecessors(2);
        CHECK(pred_2.size() == 2);

        const auto& pred_3 = graph.get_predecessors(3);
        CHECK(pred_3.size() == 1);
        CHECK(pred_3[0].i == 2);
        CHECK(pred_3[0].w == 3);
    }

    SUBCASE("clear()") {
        sparse_weighted_bigraph graph(5);
        graph.add_edge(0, 1);
        graph.add_edge(1, 2);
        graph.clear();
        CHECK(graph.empty());
        CHECK(graph.get_num_vertices() == 0);
    }
}
#endif
