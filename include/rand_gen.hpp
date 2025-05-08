#ifndef INCLUDE_RAND_GEN_HPP
#define INCLUDE_RAND_GEN_HPP

#include <mutex>
#include <random>

class rand_gen
{
  public:
    // Static method to get the singleton instance
    static rand_gen& get_instance() {
        // Meyers Singleton: thread-safe in C++11 and later
        static rand_gen instance;
        return instance;
    }

    // Generate a random integer within a specified range (inclusive)
    int generate_random_integer(int min, int max) {
        std::lock_guard<std::mutex> lock(m_mutex);  // Thread safety
        std::uniform_int_distribution<> distribution(min, max);
        return distribution(m_generator);
    }

    // Generate a random double within a specified range [0, 1)
    double generate_random_double() {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::uniform_real_distribution<> distribution(0.0, 1.0);
        return distribution(m_generator);
    }

    // Generate a random double within a specified range [min, max)
    double generate_random_double(double min, double max) {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::uniform_real_distribution<> distribution(min, max);
        return distribution(m_generator);
    }

    rand_gen(rand_gen&&) = delete;
    rand_gen& operator=(rand_gen&&) = delete;
    rand_gen(const rand_gen&) = delete;
    rand_gen& operator=(const rand_gen&) = delete;
    ~rand_gen() = default;

  private:
    // Private constructor to prevent direct instantiation
    rand_gen()
        : m_generator(
              std::random_device {}())  // Seed with a non-deterministic source
    {}

    // Random number engine (Mersenne Twister is a good choice)
    std::mt19937 m_generator;
    // Mutex for thread safety
    std::mutex m_mutex;
};
#endif  // INCLUDE_RAND_GEN_HPP
