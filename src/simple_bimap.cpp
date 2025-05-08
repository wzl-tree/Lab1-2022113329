#include <algorithm>

#include "simple_bimap.hpp"

using index_type = simple_bimap::index_type;

bool simple_bimap::add(const std::string& str) {
    auto [iter, inserted] = m_fwd_map.try_emplace(str, size());
    if (!inserted) {
        return false;
    }
    m_inv_map.push_back(iter->first);
    return true;
}

bool simple_bimap::add(std::string&& str) {
    auto [iter, inserted] = m_fwd_map.try_emplace(std::move(str), size());
    if (!inserted) {
        return false;
    }
    m_inv_map.push_back(iter->first);
    return true;
}

simple_bimap::opt_index simple_bimap::try_get_index(
    std::string_view str) const {
    auto it = m_fwd_map.find(str);
    if (it != m_fwd_map.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::optional<std::string_view> simple_bimap::try_get_string(
    index_type index) const {
    if (index >= 0 && static_cast<size_t>(index) < m_inv_map.size()) {
        return std::string_view(m_inv_map[static_cast<size_t>(index)]);
    }
    return std::nullopt;
}

// --- Doctest Test Cases ---

#ifdef ENABLE_DOCTEST_IN_LIBRARY
#    include "doctest/doctest.h"

TEST_CASE("simple_bimap basic functionality") {
    simple_bimap map;

    // Initially empty
    CHECK(map.empty());
    CHECK(map.size() == 0);

    // Add first element
    bool result1 = map.add("apple");
    CHECK(result1);
    CHECK(!map.empty());
    CHECK(map.size() == 1);
    CHECK(map.contains("apple"));
    CHECK(!map.contains("banana"));

    // Add second element
    bool result2 = map.add("banana");
    CHECK(result2);
    CHECK(map.size() == 2);
    CHECK(map.contains("banana"));

    // Add third element
    bool result3 = map.add("cherry");
    CHECK(result3);
    CHECK(map.size() == 3);
    CHECK(map.contains("cherry"));

    // Add existing element - should return existing index
    bool result4 = map.add("apple");
    CHECK(!result4);
    CHECK(map.size() == 3);  // Size should not change
}

TEST_CASE("simple_bimap try_get_index") {
    simple_bimap map;
    map.add("apple");  // index 0
    map.add("banana");  // index 1
    map.add("cherry");  // index 2

    // Test existing strings
    auto opt_idx_apple = map.try_get_index("apple");
    CHECK(opt_idx_apple.has_value());
    CHECK(opt_idx_apple.value() == 0);

    auto opt_idx_banana = map.try_get_index("banana");
    CHECK(opt_idx_banana.has_value());
    CHECK(opt_idx_banana.value() == 1);

    auto opt_idx_cherry = map.try_get_index("cherry");
    CHECK(opt_idx_cherry.has_value());
    CHECK(opt_idx_cherry.value() == 2);

    // Test non-existing string
    auto opt_idx_date = map.try_get_index("date");
    CHECK(!opt_idx_date.has_value());
}

TEST_CASE("simple_bimap try_get_string") {
    simple_bimap map;
    map.add("apple");  // index 0
    map.add("banana");  // index 1
    map.add("cherry");  // index 2

    // Test valid indices
    auto opt_str_0 = map.try_get_string(0);
    CHECK(opt_str_0.has_value());
    CHECK(opt_str_0.value() == "apple");

    auto opt_str_1 = map.try_get_string(1);
    CHECK(opt_str_1.has_value());
    CHECK(opt_str_1.value() == "banana");

    auto opt_str_2 = map.try_get_string(2);
    CHECK(opt_str_2.has_value());
    CHECK(opt_str_2.value() == "cherry");

    // Test invalid indices
    auto opt_str_neg1 = map.try_get_string(-1);
    CHECK(!opt_str_neg1.has_value());

    auto opt_str_3 = map.try_get_string(3);  // Size is 3, indices 0, 1, 2
    CHECK(!opt_str_3.has_value());
}

TEST_CASE("simple_bimap contains") {
    simple_bimap map;
    map.add("apple");
    map.add("banana");

    CHECK(map.contains("apple"));
    CHECK(map.contains("banana"));
    CHECK(!map.contains("cherry"));
    CHECK(!map.contains("date"));
}

TEST_CASE("simple_bimap size and empty") {
    simple_bimap map;
    CHECK(map.empty());
    CHECK(map.size() == 0);

    map.add("one");
    CHECK(!map.empty());
    CHECK(map.size() == 1);

    map.add("two");
    CHECK(!map.empty());
    CHECK(map.size() == 2);

    map.add("one");  // Add existing
    CHECK(!map.empty());
    CHECK(map.size() == 2);  // Size should not change
}

TEST_CASE("simple_bimap clear") {
    simple_bimap map;
    map.add("apple");
    map.add("banana");
    map.add("cherry");

    CHECK(map.size() == 3);
    CHECK(!map.empty());

    map.clear();

    CHECK(map.empty());
    CHECK(map.size() == 0);
    CHECK(!map.contains("apple"));
    CHECK(!map.try_get_index("banana").has_value());
    CHECK(!map.try_get_string(0).has_value());
}

TEST_CASE("simple_bimap get_strings") {
    simple_bimap map;
    map.add("apple");
    map.add("banana");
    map.add("cherry");

    auto strings = map.get_strings();

    CHECK(strings.size() == 3);

    // Convert to vector of strings for easier comparison and sorting
    std::vector<std::string> string_copies;
    for (const auto& sv : strings) {
        string_copies.emplace_back(sv);
    }

    // Sort to make comparison order-independent
    std::ranges::sort(string_copies);

    CHECK(string_copies[0] == "apple");
    CHECK(string_copies[1] == "banana");
    CHECK(string_copies[2] == "cherry");

    // Check that adding an existing string doesn't affect get_strings result
    // size
    map.add("apple");
    auto strings_after_add = map.get_strings();
    CHECK(strings_after_add.size() == 3);
}
#endif
