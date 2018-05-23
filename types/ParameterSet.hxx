#ifndef FHICLCPP_SIMPLE_TYPES_PARAMETER_SET_HXX_SEEN
#define FHICLCPP_SIMPLE_TYPES_PARAMETER_SET_HXX_SEEN

#include "types/Base.hxx"
#include "types/exception.hxx"

#include "string_parsers/from_string.hxx"
#include "string_parsers/md5.hxx"
#include "string_parsers/to_string.hxx"
#include "string_parsers/traits.hxx"

#include <cstdint>
#include <iomanip>
#include <memory>

namespace fhicl {

typedef uint32_t ParameterSetID;
typedef std::string key_t;

class ParameterSet : public Base {
  std::map<std::string, std::shared_ptr<Base>> internal_rep;

  ParameterSet *parent;
  ParameterSet *prolog;

  void from(std::string const &str);

  bool valid_key(key_t const &key) { return true; }

  bool check_key(key_t const &key, bool throw_on_not_exist = false) {
    if (!key.size()) {
      throw null_key();
    }
    if (!valid_key(key)) {
      throw invalid_key() << "[ERROR]: Invalid key " << std::quoted(key);
    }
    if (internal_rep.find(key) == internal_rep.end()) {
      if (throw_on_not_exist) {
        throw nonexistant_key() << "[ERROR]: Key " << std::quoted(key)
                                << " does not exist in parameter set.";
      }
      return false;
    }
    return true;
  }

  std::string get_fhicl_category(key_t const &key) {
    check_key(key, true);
    if (is_key_to_table(key)) {
      return "table";
    }
    if (is_key_to_table(key)) {
      return "sequence";
    }
    return "atom";
  }

public:
  ParameterSet(std::string const &str) : internal_rep(), parent(nullptr) {
    from(str);
  }
  ParameterSet(ParameterSet const &other)
      : internal_rep(other.internal_rep), parent(other.parent) {}
  ParameterSet() : internal_rep(), parent(nullptr) {}

  bool is_empty() { return internal_rep.size(); }
  ParameterSetID id() {
    std::string md = md5(to_string());
    ParameterSetID ID = 1;
    while (md.size()) {
      ID *= string_parsers::str2T<ParameterSetID>(std::string("0x") +
                                                  md.substr(0, 4));
      md = md.substr(4);
    }
    return ID;
  }

  std::string to_string() {
    std::stringstream ss("");
    for (auto ip_it = internal_rep.begin(); ip_it != internal_rep.end();
         ++ip_it) {
      std::shared_ptr<ParameterSet> ps =
          std::dynamic_pointer_cast<ParameterSet>(ip_it->second);
      if (ps) {
        ss << ip_it->first << ": { " << ip_it->second->to_string() << " } ";
      } else {
        ss << ip_it->first << ": " << ip_it->second->to_string() << " ";
      }
    }
    return ss.str();
  }
  std::string to_compact_string() {
    std::stringstream ss("");
    for (auto ip_it = internal_rep.begin(); ip_it != internal_rep.end();
         ++ip_it) {
      std::shared_ptr<ParameterSet> ps =
          std::dynamic_pointer_cast<ParameterSet>(ip_it->second);
      if (ps) {
        ss << ip_it->first << ": @id::" << ps->id() << " ";
      } else {
        ss << ip_it->first << ": " << ip_it->second->to_string() << " ";
      }
    }
    return ss.str();
  }
  std::string to_indented_string(size_t indent_level = 0) {
    std::stringstream ss("");
    for (auto ip_it = internal_rep.begin(); ip_it != internal_rep.end();
         ++ip_it) {
      for (size_t i_it = 0; i_it < indent_level; ++i_it) {
        ss << " ";
      }
      if (is_key_to_atom(ip_it->first)) {
        ss << ip_it->first << ": " << ip_it->second->to_indented_string(0)
           << std::endl;
      } else if (is_key_to_sequence(ip_it->first)) {
        ss << ip_it->first << ": "
           << ip_it->second->to_indented_string(indent_level +
                                                ip_it->first.size() + 2);
      } else {
        ss << ip_it->first << ": {" << std::endl
           << ip_it->second->to_indented_string(indent_level +
                                                ip_it->first.size() + 4);
        for (size_t i_it = 0; i_it < (indent_level + ip_it->first.size() + 2);
             ++i_it) {
          ss << " ";
        }
        ss << "}";
      }
    }
    return ss.str();
  }
  std::vector<key_t> get_names() {
    std::vector<key_t> names;
    for (auto ip_it = internal_rep.begin(); ip_it != internal_rep.end();
         ++ip_it) {
      names.push_back(ip_it->first);
    }
    return names;
  }
  std::vector<key_t> get_pset_names() {
    std::vector<key_t> names;
    for (auto ip_it = internal_rep.begin(); ip_it != internal_rep.end();
         ++ip_it) {
      std::shared_ptr<ParameterSet> ps =
          std::dynamic_pointer_cast<ParameterSet>(ip_it->second);
      if (ps) {
        names.push_back(ip_it->first);
      }
    }
    return names;
  }
  std::string get_src_info(key_t const &key) { return "dummy.fcl:null"; }
  bool has_key(key_t const &key) { return check_key(key); }
  bool is_key_to_atom(key_t const &key) {
    if (!check_key(key)) {
      return false;
    }
    std::shared_ptr<Atom> atm =
        std::dynamic_pointer_cast<Atom>(internal_rep[key]);
    return bool(atm);
  }
  bool is_key_to_sequence(key_t const &key);
  bool is_key_to_table(key_t const &key) {
    if (!check_key(key)) {
      return false;
    }
    std::shared_ptr<ParameterSet> ps =
        std::dynamic_pointer_cast<ParameterSet>(internal_rep[key]);
    return bool(ps);
  }
  // get table
  template <typename T>
  typename std::enable_if<std::is_same<T, ParameterSet>::value, T>::type
  get(key_t const &key) {
    check_key(key, true);
    if (!is_key_to_table(key)) {
      throw wrong_fhicl_category()
          << "[ERROR]: Attempted to retrieve key: " << std::quoted(key)
          << " as a fhicl table (fhicl::ParameterSet), but it corresponds to a "
          << std::quoted(get_fhicl_category(key));
    }
    std::shared_ptr<ParameterSet> ps =
        std::dynamic_pointer_cast<ParameterSet>(internal_rep[key]);
    return *ps;
  }
  // get other
  template <typename T>
  typename std::enable_if<!std::is_same<T, ParameterSet>::value, T>::type
  get(key_t const &key) {
    check_key(key, true);

    if (is_seq<T>::value && !is_key_to_sequence(key)) {
      throw wrong_fhicl_category()
          << "[ERROR]: Attempted to retrieve key: " << std::quoted(key)
          << " as a fhicl sequence(" << is_seq<T>::get_sequence_type()
          << "), but it corresponds to a "
          << std::quoted(get_fhicl_category(key));
    }
    if (!is_seq<T>::value && is_key_to_sequence(key)) {
      throw wrong_fhicl_category()
          << "[ERROR]: Attempted to retrieve key: " << std::quoted(key)
          << " as a fhicl atom, but it corresponds to a fhicl sequence";
    }

    if (is_seq<T>::value && is_key_to_table(key)) {
      throw wrong_fhicl_category()
          << "[ERROR]: Attempted to retrieve key: " << std::quoted(key)
          << " as a fhicl sequence(" << is_seq<T>::get_sequence_type()
          << "), but it corresponds to a fhicl table (ParameterSet)";
    }
    if (!is_seq<T>::value && is_key_to_table(key)) {
      throw wrong_fhicl_category()
          << "[ERROR]: Attempted to retrieve key: " << std::quoted(key)
          << " as a fhicl atom, but it corresponds to a fhicl table "
             "(ParameterSet)";
    }

    return string_parsers::str2T<T>(internal_rep[key]->to_string());
  };

  template <typename T> T get(key_t const &key, T def) {
    try {
      return get<T>(key);
    } catch (fhicl::string_parsers::fhicl_cpp_simple_except &e) { // parser fail
      return def;
    } catch (fhicl::fhicl_cpp_simple_except &e) { // type fail
      return def;
    } catch (std::exception &e) {
      throw bizare_error()
          << "[ERROR]: Caught unexpected exception in ParameterSet::get: "
          << std::quoted(e.what());
    }
  };

  template <typename T> bool get_if_present(key_t const &key, T &rtn) {
    if (!check_key(key)) {
      return false;
    }
    try {
      rtn = string_parsers::str2T<T>(internal_rep[key]->to_string());
    } catch (fhicl::string_parsers::fhicl_cpp_simple_except &e) { // parser fail
      return false;
    } catch (fhicl::fhicl_cpp_simple_except &e) { // type fail
      return false;
    } catch (std::exception &e) {
      throw bizare_error()
          << "[ERROR]: Caught unexpected exception in ParameterSet::get: "
          << std::quoted(e.what());
    }
    return true;
  };

  // put table
  template <typename T>
  typename std::enable_if<std::is_same<T, ParameterSet>::value, void>::type
  put(key_t const &key, T const &value) {
    if (has_key(key)) {
      throw cant_insert() << "[ERROR]: Cannot put with key: "
                          << std::quoted(key) << " as that key already exists.";
    }
    internal_rep[key] = std::make_shared<T>(value);
  }
  // put sequence
  template <typename T>
  typename std::enable_if<
      !std::is_same<T, ParameterSet>::value && is_seq<T>::value, void>::type
  put(key_t const &key, T const &value);
  // put atom
  template <typename T>
  typename std::enable_if<
      !std::is_same<T, ParameterSet>::value && !is_seq<T>::value, void>::type
  put(key_t const &key, T const &value) {
    if (has_key(key)) {
      throw cant_insert() << "[ERROR]: Cannot put with key: "
                          << std::quoted(key) << " as that key already exists.";
    }
    internal_rep[key] = std::make_shared<Atom>(string_parsers::T2Str<T>(value));
  }
};

} // namespace fhicl

#endif
