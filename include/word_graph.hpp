#ifndef WORD_GRAPH_HPP
#define WORD_GRAPH_HPP

#include <functional>
#include <map>
#include <optional>
#include <ostream>
#include <set>
#include <span>
#include <vector>

// Forward declarations or includes for your conceptual types
// (You would need to define these classes/structs elsewhere)
#include "rand_gen.hpp"
#include "simple_bimap.hpp"
#include "sw_bigraph.hpp"

class random_walk_iterator;

// Define the necessary types used by the graph structure
class word_graph
{
  public:
    using weight = int;
    using index_type =
        int;  // Represents the index used internally for vertices
    using opt_index = compact_optional<index_type, -1>;

    using edge = sparse_weighted_bigraph::edge;  // Represents a connection to
                                                 // another vertex with a weight
  private:
    class random_walk_iterator
    {
        const word_graph& graph;
        bool finished = false;
        std::set<std::pair<index_type, index_type>> visited_edges;
        int current = 0;

      public:
        explicit random_walk_iterator(const word_graph& graph)
            : graph(graph) {
            auto& rg = rand_gen::get_instance();
            auto n = graph.get_num_vertices();
            if (n == 0) {
                finished = true;
            }
            current = rg.generate_random_integer(0, n - 1);
        }

        std::optional<std::string_view> get();

        std::optional<std::string_view> next();
    };

    // Conceptual members representing the core data structures
    // (These types need to be defined elsewhere)

    // Maps between string representations of words and their graph_index
    // Internally handles m_fwd_map and m_inv_map
    simple_bimap m_bimap;

    // Stores the graph's edges using adjacency lists
    // Internally handles m_adj_list and possibly m_num_vertices
    sparse_weighted_bigraph m_graph;

    // Private helper methods (likely delegate to 'map' or 'graph')
    opt_index try_get_index(std::string_view vert) const {
        return m_bimap.try_get_index(vert);
    };

    void print_edges(std::ostream& os,
                     std::function<std::string(index_type, index_type, weight)>
                         formatter) const;

  public:
    // Constructor
    word_graph() =
        default;  // Default constructor might initialize 'map' and 'graph'

    // Public interface methods (likely delegate to 'map' and/or 'graph')
    bool has_vertex(std::string_view vert) const {
        return m_bimap.contains(vert);
    };

    // Note: Add vertex methods would involve both 'map' and 'graph'
    bool add_vertex(const std::string& vert);
    bool add_vertex(std::string&& vert);

    std::span<const std::string_view> get_vertices() const {
        return m_bimap.get_strings();
    };

    // Add an edge to the graph (involves looking up indices via 'map', adding
    // edge via 'graph')
    bool add_edge(std::string_view from, std::string_view to);

    // Get the number of vertices (might get from 'map' or 'graph')
    int get_num_vertices() const { return static_cast<int>(m_bimap.size()); };

    // Get the number of vertices (might get from 'map' or 'graph')
    int get_num_edges() const {
        return static_cast<int>(m_graph.get_num_edges());
    };

    bool empty() const {
        return m_bimap.empty();
    };  // Check if 'map' and 'graph' are empty

    void clear() {
        m_bimap.clear();
        m_graph.clear();
    };  // Clear both 'map' and 'graph'

    // These print methods would use data from 'map' and 'graph'
    void print_graph() const;
    void output_graphviz(std::ostream& os) const;

    std::optional<std::vector<std::string_view>> get_bridge_words(
        std::string_view from, std::string_view to) const;

    bool find_shortest_path(std::string_view src_str,
                            std::string_view dst_str) const;

    void page_rank() const;

    random_walk_iterator random_walk() const;
};

#endif  // WORD_GRAPH_HPP
