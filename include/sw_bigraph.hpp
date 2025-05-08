#ifndef SPARSE_WEIGHTED_BIGRAPH_HPP
#define SPARSE_WEIGHTED_BIGRAPH_HPP

#include <cassert>
#include <map>
#include <optional>
#include <span>
#include <vcruntime.h>
#include <vector>

#include "compact_optional.hpp"

class sparse_weighted_bigraph
{
  public:
    using index_type = int;
    using weight = int;

    using opt_weight = compact_optional<weight, -1>;

    struct edge
    {
        index_type i;
        weight w;
    };

  private:
    std::vector<std::vector<edge>> m_adj_list;  // Successors
    std::vector<std::vector<edge>> m_rev_adj_list;  // Predecessors
    int m_num_vertices = 0;
    int m_num_edges = 0;

    std::span<edge> get_edges_mut(index_type vert, bool successors) {
        assert(vert >= 0 && vert < m_num_vertices);
        auto casted_vert = static_cast<size_t>(vert);
        return {successors ? m_adj_list[casted_vert] : m_rev_adj_list[casted_vert]};
    }

    std::span<const edge> get_edges(index_type vert, bool successors) const {
        assert(vert >= 0 && vert < m_num_vertices);
        auto casted_vert = static_cast<size_t>(vert);
        return {successors ? m_adj_list[casted_vert] : m_rev_adj_list[casted_vert]};
    }

  public:
    sparse_weighted_bigraph() = default;

    explicit sparse_weighted_bigraph(size_t n)
        : m_adj_list(n)
        , m_rev_adj_list(n)
        , m_num_vertices(static_cast<int>(n)) {}

    void resize(size_t n) {
        m_num_vertices = static_cast<int>(n);
        m_adj_list.resize(n);
        m_rev_adj_list.resize(n);
    }

    int get_num_vertices() const { return m_num_vertices; }

    int get_num_edges() const { return m_num_edges; }

    bool empty() const { return m_num_vertices == 0; }

    void clear() {
        m_num_vertices = 0;
        m_adj_list.clear();
        m_rev_adj_list.clear();
    }

    void add_vert() {
        ++m_num_vertices;
        m_adj_list.emplace_back();
        m_rev_adj_list.emplace_back();
    };

    bool add_edge(index_type from, index_type to);

    opt_weight get_weight(index_type from, index_type to) const;

    std::span<const edge> get_successors(index_type vert) const {
        return get_edges(vert, /*successors=*/true);
    }

    std::span<const edge> get_predecessors(index_type vert) const {
        return get_edges(vert, /*successors=*/false);
    }

    std::pair<std::vector<weight>, std::vector<weight>> dijkstra(
        index_type src) const;

    std::vector<double> page_rank(double damping = 0.85,
                                  int max_iter = 100,
                                  double epsilon = 1e-8) const;
};

#endif  // SPARSE_WEIGHTED_BIGRAPH_HPP
