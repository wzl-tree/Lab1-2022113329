#include <limits>
#include <queue>
#include <vector>

#include "sw_bigraph.hpp"

using weight = sparse_weighted_bigraph::weight;
using index_type = sparse_weighted_bigraph::index_type;

using distance_map = std::vector<weight>;
using predecessor_map = std::vector<index_type>;

std::pair<distance_map, predecessor_map> sparse_weighted_bigraph::dijkstra(
    index_type source_node) const {
    // Initialize the result pair.
    // distances: Stores the shortest distance found so far from the source to
    // each node. Initialized to infinity for all nodes except the source.
    //
    // predecessors: Stores the predecessor node in the shortest path from the
    // source to each node. Initialized to -1 (or some invalid index) for all
    // nodes.

    std::pair<distance_map, predecessor_map> result {
        distance_map(m_num_vertices, std::numeric_limits<weight>::max()),
        predecessor_map(m_num_vertices, -1)};  // Enable RVO

    distance_map& distances = result.first;
    predecessor_map& predecessors = result.second;

    // Define a comparison lambda for the priority queue. It compares edges
    // based on their weight
    auto compare_edges = [](const edge& lhs, const edge& rhs)
    { return lhs.w > rhs.w; };

    // Create a min-priority queue to store edges to visit, prioritized by their
    // weight.
    std::priority_queue<edge, std::vector<edge>, decltype(compare_edges)>
        min_pq(compare_edges);

    // Initialize the distance to the source node as 0 and its predecessor as
    // itself.
    distances[source_node] = 0;
    predecessors[source_node] = source_node;

    // Add the source node to the priority queue with its initial distance.
    min_pq.emplace(source_node, 0);

    // Dijkstra's algorithm main loop. Continues as long as there are nodes to
    // explore.
    while (!min_pq.empty()) {
        // Get the edge with the smallest weight from the priority queue.
        weight current_distance = min_pq.top().w;
        index_type current_node = min_pq.top().i;

        // Remove the current edge from the priority queue.
        min_pq.pop();

        // If the current distance is greater than the known shortest distance
        // to the current node, it means we have already found a shorter path to
        // this node, so we can skip it.
        if (current_distance > distances[current_node]) {
            continue;
        }

        // Iterate over all outgoing edges from the current node.
        for (const auto& out_edge : get_successors(current_node)) {
            index_type neighbor = out_edge.i;
            weight edge_weight = out_edge.w;
            weight new_distance = current_distance + edge_weight;

            // If a shorter path to the neighbor is found through the current
            // node:
            if (new_distance < distances[neighbor]) {
                // Update the shortest distance to the neighbor.
                distances[neighbor] = new_distance;
                // Update the predecessor of the neighbor to the current node.
                predecessors[neighbor] = current_node;
                // Add the neighbor to the priority queue with its new shortest
                // distance.
                min_pq.emplace(neighbor, new_distance);
            }
        }
    }

    // Return the maps containing the shortest distances and the predecessors.
    return result;
}
