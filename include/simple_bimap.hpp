#ifndef INCLUDE_SIMPLE_BIMAP_HPP
#define INCLUDE_SIMPLE_BIMAP_HPP

#include <map>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "compact_optional.hpp"

// A simple bidirectional map between std::string and contiguous int indices
class simple_bimap
{
  public:
    // Type alias for the integer index
    using index_type = int;
    using opt_index = compact_optional<index_type, -1>;

  private:
    // Map from string to index
    // Uses std::less<> for heterogeneous lookup allowing std::string_view
    std::map<std::string, index_type, std::less<>> m_fwd_map;

    // Vector from index to string copy
    // Index i corresponds to the string int_to_string_[i]
    std::vector<std::string_view> m_inv_map;

  public:
    // Constructor
    simple_bimap() = default;

    // Add a string to the bimap.
    // If the string already exists, returns its existing index.
    // If the string is new, assigns it the next available index and returns it.
    // Uses pass-by-value for efficient handling of both lvalue and rvalue
    // strings.
    bool add(const std::string& str);
    bool add(std::string&& str);

    // Try to get the index for a given string view.
    // Returns an optional index: contains the index if found, std::nullopt
    // otherwise.
    opt_index try_get_index(std::string_view str) const;

    // Try to get the string view for a given index.
    // Returns an optional string view: contains the string view if found,
    // std::nullopt otherwise.
    std::optional<std::string_view> try_get_string(index_type index) const;

    std::string_view get_string(index_type index) const {
        return m_inv_map[static_cast<size_t>(index)];
    }

    // Check if a string exists in the bimap.
    bool contains(std::string_view str) const {
        return m_fwd_map.contains(str);
    }

    // Get the total number of mappings (vertices).
    size_t size() const {
        // Both maps should have the same size
        return m_inv_map.size();
    }

    // Check if the bimap is empty.
    bool empty() const { return size() == 0; }

    // Clear all mappings from the bimap.
    void clear() {
        m_fwd_map.clear();
        m_inv_map.clear();
    }

    // Get a view of all stored strings (useful for iterating vertices)
    // The string_views are valid as long as the bimap is not modified.
    std::span<const std::string_view> get_strings() const {
        return {m_inv_map};
    }
};
#endif  // INCLUDE_INCLUDE_SIMPLE_BIMAP_HPP
