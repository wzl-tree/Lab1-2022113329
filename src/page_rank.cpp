#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <iostream>
#include <numeric>
#include <optional>
#include <ranges>
#include <span>
#include <utility>
#include <vector>

#include "sw_bigraph.hpp"

// Rewritten weighted_page_rank function using sparse_weighted_bigraph

std::vector<double> sparse_weighted_bigraph::page_rank(double damping,
                                                       int max_iter,
                                                       double epsilon) const {
    const int n = get_num_vertices();  // Get number of vertices from the graph
    if (n == 0) {
        return {};  // Return empty vector for an empty graph
    }

    std::vector<double> pr_current(
        n, 1.0 / n);  // Current PR values, initialized uniformly
    std::vector<weight> out_weight_sums(
        n, 0);  // Sum of outgoing edge weights for each node
    std::vector<index_type> dangling_nodes;  // List of nodes with no outgoing
                                             // edges (sum of weights is 0)

    // Preprocessing phase: Calculate sum of outgoing edge weights and identify
    // dangling nodes
    for (int u = 0; u < n; ++u) {
        // Get all outgoing edges' weight
        auto successor_weights = get_successors(u)
            | std::views::transform([](auto& e) { return (e.w); });
        if (successor_weights.empty()) {  // Node with no outgoing edges
            // is a dangling node
            dangling_nodes.push_back(u);
            continue;
        }
        int sum =
            std::reduce(successor_weights.begin(), successor_weights.end());
        out_weight_sums[u] = sum;
    }

    // Pre-calculate constant term for the PageRank formula
    const double const_term = (1.0 - damping) / n;
    std::vector<double> pr_next(n);  // Next iteration's PR values

    // PageRank Iteration Loop
    for (int iter = 0; iter < max_iter; ++iter) {
        // Initialize pr_next with the constant term for all nodes
        std::ranges::fill(pr_next, const_term);

        // First part: Distribute PageRank from non-dangling nodes
        for (index_type u = 0; u < n; ++u) {
            // Skip dangling nodes as their contribution is handled separately
            if (out_weight_sums[u] == 0) {
                continue;
            }

            // Calculate the factor to distribute from node u
            // This is PR(u) * damping / sum_of_out_weights(u)
            const double factor = damping * pr_current[u]
                / static_cast<double>(out_weight_sums[u]);

            // Distribute the factor to all successor nodes v
            // Use graph.get_successors() to iterate over outgoing edges
            for (const auto& edge : get_successors(u)) {
                // Add weighted contribution to successor's PR
                pr_next[edge.i] += factor * static_cast<double>(edge.w);
            }
        }

        // Second part: Handle contribution from dangling nodes
        // Dangling nodes distribute their entire PR equally among all nodes
        if (!dangling_nodes.empty()) {
            double dangling_sum = 0;
            // Sum the PR of all dangling nodes
            for (int u : dangling_nodes) {
                dangling_sum += pr_current[u];
            }
            // Calculate the contribution from all dangling nodes to each node
            const double dangling_contrib = damping * dangling_sum / n;

            // Add this contribution to every node's PR
            for (int v = 0; v < n; ++v) {
                pr_next[v] += dangling_contrib;
            }
        }

        // Calculate the total difference between the current and next PR
        // vectors This is used to check for convergence
        double diff = 0.0;
        for (int v = 0; v < n; ++v) {
            diff += std::abs(pr_next[v] - pr_current[v]);
        }

        // Check for convergence
        if (diff < epsilon) {
            break;  // Stop iterating if the change is below the threshold
        }

        // Prepare for the next iteration:
        // Swap pr_current and pr_next to avoid copying the large vectors
        std::swap(pr_current, pr_next);
    }

    // Return the final converged (or max_iter reached) PageRank values
    return pr_current;
}
