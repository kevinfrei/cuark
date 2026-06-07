module;

#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <crow/json.h>

export module ts_cpp_idl.crow_support;

using std::map;
using std::optional;
using std::set;
using std::string;
using std::string_view;
using std::tuple;
using std::unordered_map;
using std::unordered_set;
using std::vector;

using crow::json::rvalue;
using crow::json::wvalue;

/****
Conversion to JSON stuff
****/

export {
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
    static inline wvalue process(const T& value) {
      return wvalue(value);
    }
  };

  template <typename T>
  inline std::enable_if_t<!is_enum_class_v<T>, wvalue> to_json(const T& value) {
    return impl_to_json<T>::process(value);
  }

  template <typename T>
  inline std::enable_if_t<is_enum_class_v<T>, wvalue> to_json(const T value) {
    return wvalue(static_cast<std::underlying_type_t<T>>(value));
  }

  template <>
  struct impl_to_json<uint64_t> {
    static inline wvalue process(const uint64_t& value) {
      wvalue v;
      v["@dataType"] = "freik.BigInt";
      v["@dataValue"] = std::to_string(value);
      return v;
    }
  };

  template <>
  struct impl_to_json<int64_t> {
    static inline wvalue process(const int64_t& value) {
      wvalue v;
      v["@dataType"] = "freik.BigInt";
      v["@dataValue"] = std::to_string(value);
      return v;
    }
  };

  // A little extra work for string_view's:
  template <>
  struct impl_to_json<string_view> {
    static inline wvalue process(string_view value) {
      return wvalue(string{value});
    }
  };

  // Vector<T> specialization
  template <typename T>
  struct impl_to_json<vector<T>> {
    static inline wvalue process(const vector<T>& value) {
      wvalue vec{vector<wvalue>()};
      for (uint32_t i = 0; i < value.size(); i++) {
        vec[i] = to_json(value[i]);
      }
      return vec;
    }
  };

  // Handle char's specifically. Hurray dumb Javascript?
  template <>
  struct impl_to_json<char> {
    static inline wvalue process(char value) {
      return wvalue(string(1, value));
    }
  };

  template <typename... Args>
  struct impl_to_json<tuple<Args...>> {
    using tup_type = tuple<Args...>;
    // Tuples in JSON are "just" arrays with the right size & types.
    // Doing that is...complicated.
    template <size_t Index, typename T>
    static inline void element_helper(const tup_type& tuple,
                                      wvalue& container) {
      container[Index] = to_json(std::get<Index>(tuple));
    }
    // This is a compile time 'call the helper for each type' thing.
    template <size_t... Is>
    static inline void recurse_helper(const tup_type& tuple,
                                      wvalue& json_list,
                                      std::index_sequence<Is...>) {
      ((element_helper<Is, std::tuple_element_t<Is, tup_type>>(tuple,
                                                               json_list)),
       ...);
    }
    // make_index_sequence is the magic to do something different per tuple-item
    static inline wvalue process(const tuple<Args...>& value) {
      wvalue vec{vector<wvalue>()};
      recurse_helper(value, vec, std::make_index_sequence<sizeof...(Args)>{});
      return vec;
    }
  };

  // I didn't need this for Windows, but I do for Linux/Mac because the map
  // elem type is std::tuple on one, but a std::pair on the other
  template <typename Iter>
  inline wvalue impl_vec_pair_to_json(const Iter& begin, const Iter& end) {
    vector<wvalue> vec;
    for (auto it = begin; it != end; ++it) {
      wvalue pair{vector<wvalue>()};
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
  struct impl_to_json<map<K, V>> {
    static inline wvalue process(const map<K, V>& value) {
      wvalue v;
      v["@dataType"] = "freik.Map";
      v["@dataValue"] = impl_vec_pair_to_json(value.begin(), value.end());
      return v;
    }
  };

  template <typename K, typename V>
  struct impl_to_json<std::unordered_map<K, V>> {
    static inline wvalue process(const unordered_map<K, V>& value) {
      wvalue v;
      v["@dataType"] = "freik.Map";
      v["@dataValue"] = impl_vec_pair_to_json(value.begin(), value.end());
      return v;
    }
  };

  // My pickling framework sends JS Sets as this:
  // {"@dataType":"freik.Set","@dataValue":["a", "c", "b"]}
  template <typename T>
  struct impl_to_json<unordered_set<T>> {
    static inline wvalue process(const unordered_set<T>& value) {
      wvalue v;
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
    static inline wvalue process(const set<T>& value) {
      wvalue v;
      v["@dataType"] = "freik.Set";
      std::vector<T> flat;
      flat.reserve(value.size());
      flat.assign(value.begin(), value.end());
      v["@dataValue"] = to_json(flat);
      return v;
    }
  };

  template <typename T>
  struct impl_to_json<optional<T>> {
    static inline wvalue process(const optional<T>& value) {
      if (value.has_value()) {
        return to_json(*value);
      } else {
        return wvalue(nullptr);
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
    static inline optional<T> process(const rvalue&) {
      return std::nullopt;
    }
  };

  // Anything that doesn't either fully specialize this function,
  // or partially (or fully...) specialize the above struct gets std::nullopt.
  template <typename T>
  inline optional<T> from_json(const rvalue& json) {
    return impl_from_json<T>::process(json);
  }

  // Chars are a little weird, cuz, Javascript
  template <>
  inline optional<char> from_json<char>(const rvalue& json) {
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
  inline optional<uint8_t> from_json<uint8_t>(const rvalue& json) {
    if (json.nt() != crow::json::num_type::Unsigned_integer) {
      return std::nullopt;
    }
    return static_cast<uint8_t>(json.u());
  }

  template <>
  inline optional<int8_t> from_json<int8_t>(const rvalue& json) {
    if (json.nt() == crow::json::num_type::Floating_point) {
      return std::nullopt;
    }
    return static_cast<int8_t>(json.u());
  }

  template <>
  inline optional<uint16_t> from_json<uint16_t>(const rvalue& json) {
    if (json.nt() != crow::json::num_type::Unsigned_integer) {
      return std::nullopt;
    }
    return static_cast<uint16_t>(json.u());
  }

  template <>
  inline optional<int16_t> from_json<int16_t>(const rvalue& json) {
    if (json.nt() == crow::json::num_type::Floating_point) {
      return std::nullopt;
    }
    return static_cast<int16_t>(json.u());
  }

  template <>
  inline optional<uint32_t> from_json<uint32_t>(const rvalue& json) {
    if (json.nt() != crow::json::num_type::Unsigned_integer) {
      return std::nullopt;
    }
    return static_cast<uint32_t>(json.u());
  }

  template <>
  inline optional<int32_t> from_json<int32_t>(const rvalue& json) {
    if (json.nt() == crow::json::num_type::Floating_point) {
      return std::nullopt;
    }
    return static_cast<int32_t>(json.u());
  }

  template <>
  inline optional<uint64_t> from_json<uint64_t>(const rvalue& json) {
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
    string str = dataValue.s();
    return std::stoull(str);
  }

  template <>
  inline optional<int64_t> from_json<int64_t>(const rvalue& json) {
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
    string str = dataValue.s();
    return std::stoll(str);
  }

  template <>
  inline optional<float> from_json<float>(const rvalue& json) {
    if (json.t() != crow::json::type::Number) {
      return std::nullopt;
    }
    return static_cast<float>(json.d());
  }

  template <>
  inline optional<double> from_json<double>(const rvalue& json) {
    if (json.t() != crow::json::type::Number) {
      return std::nullopt;
    }
    return static_cast<double>(json.d());
  }

  template <>
  inline optional<bool> from_json<bool>(const rvalue& json) {
    if (json.t() != crow::json::type::True &&
        json.t() != crow::json::type::False) {
      return std::nullopt;
    }
    return json.t() == crow::json::type::True;
  }

  template <>
  inline optional<std::string> from_json<std::string>(const rvalue& json) {
    if (json.t() != crow::json::type::String) {
      return std::nullopt;
    }
    return string{json.s()};
  }

  // std::vector specialization:
  template <typename T>
  struct impl_from_json<vector<T>> {
    using value_type = T;
    static inline optional<vector<value_type>> process(const rvalue& json) {
      if (json.t() != crow::json::type::List) {
        return std::nullopt;
      }
      vector<value_type> vec;
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
  struct impl_from_json<tuple<Args...>> {
   private:
    using tup_type = tuple<Args...>;
    template <size_t Index, typename T>
    static inline optional<T> element_helper(tup_type& tuple,
                                             bool& failed,
                                             const rvalue& container) {
      optional<T> res = from_json<T>(container[Index]);
      failed = failed || !res.has_value();
      if (res.has_value()) {
        std::get<Index>(tuple) = res.value();
      }
      return res;
    }

    template <size_t... Is>
    static inline optional<tup_type> recurse(const rvalue& container,
                                             std::index_sequence<Is...>) {
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
    static inline optional<tup_type> process(const rvalue& json) {
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
    static inline optional<SetType<Elem>> process(const rvalue& json) {
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
  struct impl_from_json<set<Elem>> {
    static inline optional<set<Elem>> process(const rvalue& json) {
      return impl_set_helper_from_json<set, Elem>::process(json);
    }
  };

  // std::unordered_set specialization
  template <typename Elem>
  struct impl_from_json<unordered_set<Elem>> {
    static inline optional<unordered_set<Elem>> process(const rvalue& json) {
      return impl_set_helper_from_json<unordered_set, Elem>::process(json);
    }
  };

  // std::map/unordered specialization helper:
  template <template <typename...> class MapType, typename K, typename V>
  struct impl_map_helper_from_json {
    static inline optional<MapType<K, V>> process(const rvalue& json) {
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
  struct impl_from_json<map<K, V>> {
    static inline optional<map<K, V>> process(const rvalue& json) {
      return impl_map_helper_from_json<map, K, V>::process(json);
    }
  };

  // std::unordered_set specialization
  template <typename K, typename V>
  struct impl_from_json<unordered_map<K, V>> {
    static inline optional<unordered_map<K, V>> process(const rvalue& json) {
      return impl_map_helper_from_json<unordered_map, K, V>::process(json);
    }
  };

  // Enum's, specifically for my gen'ed enum types, which include an
  // 'is_valid' free function overload
  template <typename T>
  struct impl_from_json<T, std::enable_if_t<is_enum_class_v<T>>> {
    static inline optional<T> process(const rvalue& json) {
      using IntType = std::underlying_type_t<T>;
      optional<IntType> underlyingValue = from_json<IntType>(json);
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
  struct impl_from_json<optional<T>> {
    static inline optional<optional<T>> process(
        const rvalue& json) {
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
}