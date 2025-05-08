#ifndef INCLUDE_COMPACT_OPTIONAL_HPP
#define INCLUDE_COMPACT_OPTIONAL_HPP

#include <optional>

template<typename T, T NulloptValue>
class compact_optional
{
    T m_value;

  public:
    constexpr compact_optional()
        : m_value {NulloptValue} {}

    constexpr compact_optional(T value)  // By design
        : m_value {value} {}

    constexpr compact_optional(std::nullopt_t)
        : m_value {NulloptValue} {}

    constexpr bool has_value() const { return m_value != NulloptValue; }

    constexpr T& value() { return m_value; }

    explicit constexpr operator bool() const { return m_value != NulloptValue; }

    explicit constexpr operator T() const { return m_value; }

    constexpr T& operator*() noexcept { return m_value; }

    constexpr T& operator->() noexcept { return m_value; }
};

#endif  // INCLUDE_COMPACT_OPTIONAL_HPP
