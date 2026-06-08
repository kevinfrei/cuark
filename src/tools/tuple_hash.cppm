module;

export module tools.tuple_hash;

import std;

export {

  // A helper function to combine hashes
  template <typename T>
  void hash_combine(std::size_t & seed, const T& v) {
    seed ^= std::hash<T>()(v) + 999999937 + (seed << 5) + (seed >> 4);
  }

  // Custom hash struct for std::tuple
  struct TupleHash {
    template <typename... Args>
    std::size_t operator()(const std::tuple<Args...>& t) const {
      std::size_t seed = 9999991;
      // Use std::apply to unpack the tuple and combine hashes of its elements
      std::apply(
          [&](const auto&... elements) {
            (hash_combine(seed, elements), ...); // Fold expression (C++17)
          },
          t);
      return seed;
    }
  };

} // export