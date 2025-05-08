#ifndef INCLUDE_FIXED_PRIORITY_QUEUE_HPP
#define INCLUDE_FIXED_PRIORITY_QUEUE_HPP

#include <algorithm>
#include <iostream>
#include <vector>

template<typename T, typename Compare = decltype(std::less<T> {})>
class fixed_priority_queue
{
  public:
    explicit fixed_priority_queue(int capacity, Compare comp = std::less<T> {})
        : m_data(capacity)
        , m_comp(comp) {}

    void push(const T& value) {
        if (m_size < capacity()) {
            m_data[m_size] = value;
            ++m_size;
            std::push_heap(m_data.begin(), m_data.begin() + m_size, m_comp);
        } else if (value < m_data[0]) {
            std::pop_heap(m_data.begin(), m_data.end(), m_comp);
            m_data[capacity() - 1] = value;
            std::push_heap(m_data.begin(), m_data.end(), m_comp);
        }
    }

    void sort() {
        std::sort_heap(m_data.begin(), m_data.begin() + m_size, m_comp);
    }

    T top() const { return m_data[0]; }

    void pop() {
        std::pop_heap(m_data.begin(), m_data.begin() + m_size, m_comp);
        --m_size;
    }

    std::size_t size() const { return m_size; }

    std::size_t capacity() const { return m_data.size(); }

    bool empty() const { return m_size == 0; }

    std::span<const T> as_span() const { return {m_data.begin(), m_size}; }

  private:
    std::vector<T> m_data;
    std::size_t m_size = 0;
    Compare m_comp;
};

#endif  // INCLUDE_FIXED_PRIORITY_QUEUE_HPP
