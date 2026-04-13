#pragma once

#include <cstdint>
#include <iomanip>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <crow/json.h>

/****
Conversion to JSON stuff
****/

// Helper stuff I need
// bool value for an enum-class to enable JSON specialization
template <typename T>
struct is_enum_class {
  static constexpr bool value =
      std::is_enum_v<T> && !std::is_convertible_v<T, int>;
};

template <typename T>
inline constexpr bool is_enum_class_v = is_enum_class<T>::value;

template <typename T>
constexpr auto underlying_cast(T e) noexcept {
  static_assert(std::is_enum_v<T>,
                "underlying_cast can only be used with enum types.");
  return static_cast<std::underlying_type_t<T>>(e);
}

/****
Conversion to JSON stuff
****/

// By default, simple stuff can be converted to json
// The enable_if_t is to prevent this version from being used by enum classes,
// which get extra validation to prevent stupidity from accidentally leaking.
template <typename T, typename Enabled = void>
struct impl_to_json {
  static inline crow::json::wvalue process(const T& value) {
    return crow::json::wvalue(value);
  }
};

template <typename T>
inline std::enable_if_t<!is_enum_class_v<T>, crow::json::wvalue> to_json(
    const T& value) {
  return impl_to_json<T>::process(value);
}

template <typename T>
inline std::enable_if_t<is_enum_class_v<T>, crow::json::wvalue> to_json(
    const T value) {
  return crow::json::wvalue(static_cast<std::underlying_type_t<T>>(value));
}

// A little extra work for string constants:

// Function to manually escape a string for JSON
inline std::string escape_json_string(std::string_view sv) {
  std::ostringstream o;
  for (char c : sv) {
    if (c == '"') {
      o << "\\\"";
    } else if (c == '\\') {
      o << "\\\\";
    } else if (c == '\b') {
      o << "\\b";
    } else if (c == '\f') {
      o << "\\f";
    } else if (c == '\n') {
      o << "\\n";
    } else if (c == '\r') {
      o << "\\r";
    } else if (c == '\t') {
      o << "\\t";
    } else if (static_cast<unsigned char>(c) < 0x20 ||
               static_cast<unsigned char>(c) > 0x7e) {
      // Escape other control characters and non-ASCII characters as \uXXXX
      o << "\\u" << std::hex << std::setw(4) << std::setfill('0')
        << static_cast<int>(static_cast<unsigned char>(c));
    } else {
      o << c;
    }
  }
  return o.str();
}

template <>
struct impl_to_json<std::string> {
  static inline crow::json::wvalue process(const std::string& value) {
    return crow::json::wvalue(escape_json_string(value));
  }
};
template <>
struct impl_to_json<char*> {
  static inline crow::json::wvalue process(const char* value) {
    return crow::json::wvalue(escape_json_string(value));
  }
};

template <>
struct impl_to_json<uint64_t> {
  static inline crow::json::wvalue process(const uint64_t& value) {
    crow::json::wvalue v;
    v["@dataType"] = "freik.BigInt";
    v["@dataValue"] = std::to_string(value);
    return v;
  }
};

template <>
struct impl_to_json<int64_t> {
  static inline crow::json::wvalue process(const int64_t& value) {
    crow::json::wvalue v;
    v["@dataType"] = "freik.BigInt";
    v["@dataValue"] = std::to_string(value);
    return v;
  }
};

template <>
struct impl_to_json<std::string_view> {
  static inline crow::json::wvalue process(std::string_view value) {
    return crow::json::wvalue(escape_json_string(value));
  }
};

// Vector<T> specialization
template <typename T>
struct impl_to_json<std::vector<T>> {
  static inline crow::json::wvalue process(const std::vector<T>& value) {
    crow::json::wvalue vec{std::vector<crow::json::wvalue>()};
    for (uint32_t i = 0; i < value.size(); i++) {
      vec[i] = to_json(value[i]);
    }
    return vec;
  }
};

// Handle char's specifically. Hurray dumb Javascript?
template <>
struct impl_to_json<char> {
  static inline crow::json::wvalue process(char value) {
    return crow::json::wvalue(std::string(1, value));
  }
};

template <typename... Args>
struct impl_to_json<std::tuple<Args...>> {
  using tup_type = std::tuple<Args...>;
  // Tuples in JSON are "just" arrays with the right size & types.
  // Doing that is...complicated.
  template <size_t Index, typename T>
  static inline void element_helper(const tup_type& tuple,
                                    crow::json::wvalue& container) {
    container[Index] = to_json(std::get<Index>(tuple));
  }
  // This is a compile time 'call the helper for each type' thing.
  template <size_t... Is>
  static inline void recurse_helper(const tup_type& tuple,
                                    crow::json::wvalue& json_list,
                                    std::index_sequence<Is...>) {
    ((element_helper<Is, std::tuple_element_t<Is, tup_type>>(tuple, json_list)),
     ...);
  }
  // make_index_sequence is the magic to do something different per tuple-item
  static inline crow::json::wvalue process(const std::tuple<Args...>& value) {
    crow::json::wvalue vec{std::vector<crow::json::wvalue>()};
    recurse_helper(value, vec, std::make_index_sequence<sizeof...(Args)>{});
    return vec;
  }
};

// I didn't need this for Windows, but I do for Linux/Mac because the map
// elem type is std::tuple on one, but a std::pair on the other
template <typename Iter>
inline crow::json::wvalue impl_vec_pair_to_json(const Iter& begin,
                                                const Iter& end) {
  std::vector<crow::json::wvalue> vec;
  for (auto it = begin; it != end; ++it) {
    crow::json::wvalue pair{std::vector<crow::json::wvalue>()};
    auto& [first, second] = *it;
    pair[0] = to_json(first);
    pair[1] = to_json(second);
    vec.push_back(pair);
  }
  return vec;
}

// My pickling framework sends JS Maps as this:
// {"@dataType":"freik.Map","@dataValue":[["a",1],["c",2],["b",3]]}
template <typename K, typename V>
struct impl_to_json<std::map<K, V>> {
  static inline crow::json::wvalue process(const std::map<K, V>& value) {
    crow::json::wvalue v;
    v["@dataType"] = "freik.Map";
    v["@dataValue"] = impl_vec_pair_to_json(value.begin(), value.end());
    return v;
  }
};

template <typename K, typename V>
struct impl_to_json<std::unordered_map<K, V>> {
  static inline crow::json::wvalue process(
      const std::unordered_map<K, V>& value) {
    crow::json::wvalue v;
    v["@dataType"] = "freik.Map";
    v["@dataValue"] = impl_vec_pair_to_json(value.begin(), value.end());
    return v;
  }
};

// My pickling framework sends JS Sets as this:
// {"@dataType":"freik.Set","@dataValue":["a", "c", "b"]}
template <typename T>
struct impl_to_json<std::unordered_set<T>> {
  static inline crow::json::wvalue process(const std::unordered_set<T>& value) {
    crow::json::wvalue v;
    v["@dataType"] = "freik.Set";
    std::vector<T> flat;
    flat.reserve(value.size());
    flat.assign(value.begin(), value.end());
    v["@dataValue"] = to_json(flat);
    return v;
  }
};

template <typename T>
struct impl_to_json<std::set<T>> {
  static inline crow::json::wvalue process(const std::set<T>& value) {
    crow::json::wvalue v;
    v["@dataType"] = "freik.Set";
    std::vector<T> flat;
    flat.reserve(value.size());
    flat.assign(value.begin(), value.end());
    v["@dataValue"] = to_json(flat);
    return v;
  }
};

template <typename T>
struct impl_to_json<std::optional<T>> {
  static inline crow::json::wvalue process(const std::optional<T>& value) {
    if (value.has_value()) {
      return to_json(*value);
    } else {
      return crow::json::wvalue();
    }
  }
};

/****
Conversion from JSON stuff
****/

// By default, we can't read a random value from a json value
// (i.e. everything must be specialized) and it's easier to
// do this with a partial specialization of a struct :/
template <typename T, typename Enabled = void>
struct impl_from_json {
  static inline std::optional<T> process(const crow::json::rvalue&) {
    return std::nullopt;
  }
};

// Anything that doesn't either fully specialize this function,
// or partially (or fully...) specialize the above struct gets std::nullopt.
template <typename T>
inline std::optional<T> from_json(const crow::json::rvalue& json) {
  return impl_from_json<T>::process(json);
}

// Chars are a little weird, cuz, Javascript
template <>
inline std::optional<char> from_json<char>(const crow::json::rvalue& json) {
  if (json.t() != crow::json::type::String) {
    return std::nullopt;
  }
  std::string str = json.s();
  if (str.size() != 1) {
    return std::nullopt;
  }
  return static_cast<char>(str[0]);
}

template <>
inline std::optional<uint8_t> from_json<uint8_t>(
    const crow::json::rvalue& json) {
  if (json.nt() != crow::json::num_type::Unsigned_integer) {
    return std::nullopt;
  }
  return static_cast<uint8_t>(json.u());
}

template <>
inline std::optional<int8_t> from_json<int8_t>(const crow::json::rvalue& json) {
  if (json.nt() == crow::json::num_type::Floating_point) {
    return std::nullopt;
  }
  return static_cast<int8_t>(json.u());
}

template <>
inline std::optional<uint16_t> from_json<uint16_t>(
    const crow::json::rvalue& json) {
  if (json.nt() != crow::json::num_type::Unsigned_integer) {
    return std::nullopt;
  }
  return static_cast<uint16_t>(json.u());
}

template <>
inline std::optional<int16_t> from_json<int16_t>(
    const crow::json::rvalue& json) {
  if (json.nt() == crow::json::num_type::Floating_point) {
    return std::nullopt;
  }
  return static_cast<int16_t>(json.u());
}

template <>
inline std::optional<uint32_t> from_json<uint32_t>(
    const crow::json::rvalue& json) {
  if (json.nt() != crow::json::num_type::Unsigned_integer) {
    return std::nullopt;
  }
  return static_cast<uint32_t>(json.u());
}

template <>
inline std::optional<int32_t> from_json<int32_t>(
    const crow::json::rvalue& json) {
  if (json.nt() == crow::json::num_type::Floating_point) {
    return std::nullopt;
  }
  return static_cast<int32_t>(json.u());
}

template <>
inline std::optional<uint64_t> from_json<uint64_t>(
    const crow::json::rvalue& json) {
  if (json.t() != crow::json::type::Object || json.size() != 2) {
    return std::nullopt;
  }
  if (json["@dataType"].s() != "freik.BigInt") {
    return std::nullopt;
  }
  auto dataValue = json["@dataValue"];
  if (dataValue.t() != crow::json::type::String) {
    return std::nullopt;
  }
  std::string str = dataValue.s();
  return std::stoull(str);
}

template <>
inline std::optional<int64_t> from_json<int64_t>(
    const crow::json::rvalue& json) {
  if (json.t() != crow::json::type::Object || json.size() != 2) {
    return std::nullopt;
  }
  if (json["@dataType"].s() != "freik.BigInt") {
    return std::nullopt;
  }
  auto dataValue = json["@dataValue"];
  if (dataValue.t() != crow::json::type::String) {
    return std::nullopt;
  }
  std::string str = dataValue.s();
  return std::stoll(str);
}

template <>
inline std::optional<float> from_json<float>(const crow::json::rvalue& json) {
  if (json.t() != crow::json::type::Number) {
    return std::nullopt;
  }
  return static_cast<float>(json.d());
}

template <>
inline std::optional<double> from_json<double>(const crow::json::rvalue& json) {
  if (json.t() != crow::json::type::Number) {
    return std::nullopt;
  }
  return static_cast<double>(json.d());
}

template <>
inline std::optional<bool> from_json<bool>(const crow::json::rvalue& json) {
  if (json.t() != crow::json::type::True &&
      json.t() != crow::json::type::False) {
    return std::nullopt;
  }
  return json.t() == crow::json::type::True;
}

template <>
inline std::optional<std::string> from_json<std::string>(
    const crow::json::rvalue& json) {
  if (json.t() != crow::json::type::String) {
    return std::nullopt;
  }
  return std::string(json.s());
}

// std::vector specialization:
template <typename T>
struct impl_from_json<std::vector<T>> {
  using value_type = T;
  static inline std::optional<std::vector<value_type>> process(
      const crow::json::rvalue& json) {
    if (json.t() != crow::json::type::List) {
      return std::nullopt;
    }
    std::vector<value_type> vec;
    vec.reserve(json.size());
    for (size_t i = 0; i < json.size(); i++) {
      auto item = from_json<value_type>(json[i]);
      if (!item) {
        return std::nullopt;
      }
      vec.push_back(*item);
    }
    return vec;
  }
};

// std::tuple specialization:
// TODO: Explain this, as it is rather messsy.
template <typename... Args>
struct impl_from_json<std::tuple<Args...>> {
 private:
  using tup_type = std::tuple<Args...>;
  template <size_t Index, typename T>
  static inline std::optional<T> element_helper(
      tup_type& tuple, bool& failed, const crow::json::rvalue& container) {
    std::optional<T> res = from_json<T>(container[Index]);
    failed = failed || !res.has_value();
    if (res.has_value()) {
      std::get<Index>(tuple) = res.value();
    }
    return res;
  }

  template <size_t... Is>
  static inline std::optional<tup_type> recurse(
      const crow::json::rvalue& container, std::index_sequence<Is...>) {
    tup_type res;
    bool failed = false;
    ((element_helper<Is, std::tuple_element_t<Is, tup_type>>(
         res, failed, container)),
     ...);
    if (failed)
      return std::nullopt;
    return res;
  }

 public:
  static inline std::optional<tup_type> process(
      const crow::json::rvalue& json) {
    if (json.t() != crow::json::type::List) {
      return std::nullopt;
    }
    // Check exact tuple size
    if (json.size() != sizeof...(Args)) {
      return std::nullopt;
    }
    tup_type t;
    return recurse(json, std::make_index_sequence<sizeof...(Args)>{});
  }
};

// std::set/unordered_set specialization common code:
template <template <typename...> class SetType, typename Elem>
struct impl_set_helper_from_json {
  static inline std::optional<SetType<Elem>> process(
      const crow::json::rvalue& json) {
    if (json.t() != crow::json::type::Object || json.size() != 2) {
      return std::nullopt;
    }
    if (json["@dataType"].s() != "freik.Set") {
      return std::nullopt;
    }
    auto dataValue = json["@dataValue"];
    if (dataValue.t() != crow::json::type::List) {
      return std::nullopt;
    }
    SetType<Elem> m;
    for (const auto& item : dataValue) {
      auto value = from_json<Elem>(item);
      if (!value) {
        return std::nullopt;
      }
      m.insert(*value);
    }
    return m;
  }
};

// std::set specialization
template <typename Elem>
struct impl_from_json<std::set<Elem>> {
  static inline std::optional<std::set<Elem>> process(
      const crow::json::rvalue& json) {
    return impl_set_helper_from_json<std::set, Elem>::process(json);
  }
};

// std::unordered_set specialization
template <typename Elem>
struct impl_from_json<std::unordered_set<Elem>> {
  static inline std::optional<std::unordered_set<Elem>> process(
      const crow::json::rvalue& json) {
    return impl_set_helper_from_json<std::unordered_set, Elem>::process(json);
  }
};

// std::map/unordered specialization helper:
template <template <typename...> class MapType, typename K, typename V>
struct impl_map_helper_from_json {
  static inline std::optional<MapType<K, V>> process(
      const crow::json::rvalue& json) {
    if (json.t() != crow::json::type::Object || json.size() != 2) {
      return std::nullopt;
    }
    if (json["@dataType"].s() != "freik.Map") {
      return std::nullopt;
    }
    auto dataValue = json["@dataValue"];
    if (dataValue.t() != crow::json::type::List) {
      return std::nullopt;
    }
    MapType<K, V> m;
    for (const auto& item : dataValue) {
      if (item.t() != crow::json::type::List || item.size() != 2) {
        return std::nullopt;
      }
      auto key = from_json<K>(item[0]);
      auto value = from_json<V>(item[1]);
      if (!key || !value) {
        return std::nullopt;
      }
      m[*key] = *value;
    }
    return m;
  }
};

// std::map specialization
template <typename K, typename V>
struct impl_from_json<std::map<K, V>> {
  static inline std::optional<std::map<K, V>> process(
      const crow::json::rvalue& json) {
    return impl_map_helper_from_json<std::map, K, V>::process(json);
  }
};

// std::unordered_set specialization
template <typename K, typename V>
struct impl_from_json<std::unordered_map<K, V>> {
  static inline std::optional<std::unordered_map<K, V>> process(
      const crow::json::rvalue& json) {
    return impl_map_helper_from_json<std::unordered_map, K, V>::process(json);
  }
};

// Enum's, specifically for my gen'ed enum types, which include an
// 'is_valid' free function overload
template <typename T>
struct impl_from_json<T, std::enable_if_t<is_enum_class_v<T>>> {
  static inline std::optional<T> process(const crow::json::rvalue& json) {
    using IntType = std::underlying_type_t<T>;
    std::optional<IntType> underlyingValue = from_json<IntType>(json);
    if (!underlyingValue.has_value())
      return std::nullopt;
    T val = static_cast<T>(underlyingValue.value());
    // Check that the enumeration is actually defined
    if (is_valid(val))
      return val;
    return std::nullopt;
  }
};

/* Horsing around a little bit
template <typename T>
struct impl_from_json<std::optional<T>> {
  static inline std::optional<std::optional<T>> process(
      const crow::json::rvalue& json) {
    if (json.t() == crow::json::type::Null) {
      return std::nullopt;
    }
    auto val = from_json<T>(json);
    if (!val.has_value()) {
      return std::nullopt;
    }
    return val;
  }
};
*/
