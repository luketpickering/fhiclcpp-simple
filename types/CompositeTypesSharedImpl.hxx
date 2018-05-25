#ifndef FHICLCPP_SIMPLE_TYPES_COMPOSITE_TYPES_SHARED_IMPL_HXX_SEEN
#define FHICLCPP_SIMPLE_TYPES_COMPOSITE_TYPES_SHARED_IMPL_HXX_SEEN

#include "types/Atom.hxx"
#include "types/Base.hxx"
#include "types/ParameterSet.hxx"
#include "types/Sequence.hxx"
#include "types/traits.hxx"

#include "string_parsers/exception.hxx"

#include <limits>
#include <memory>

void fhicl::Sequence::from(std::string const &str) {
  if (!str.size()) {
    return;
  }
  std::string tstr = str;
  fhicl::string_parsers::trim(tstr);
  if (!fhicl::string_parsers::well_bracket_wrapped(tstr, '[', ']')) {
    throw fhicl::string_parsers::parser_fail()
        << "[ERROR]: Attempted to parse a non-array-like string ("
        << std::quoted(str) << ") as a Sequence.";

  } else {
    tstr = tstr.substr(1, tstr.size() - 2);
  }
  for (std::string const &outer_list_item :
       fhicl::string_parsers::ParseToVect<std::string>(tstr, ",", true, true)) {
    if (fhicl::string_parsers::is_table(outer_list_item)) {
      internal_rep.push_back(std::make_shared<ParameterSet>(outer_list_item));
    } else if (fhicl::string_parsers::is_sequence(outer_list_item)) {
      internal_rep.push_back(std::make_shared<Sequence>(outer_list_item));
    } else {
      internal_rep.push_back(std::make_shared<Atom>(outer_list_item));
    }
  }
}

void fhicl::ParameterSet::from(std::string const &str) {
  if (!str.size()) {
    return;
  }
  std::string tstr = str;
  fhicl::string_parsers::trim(tstr);
  if (!fhicl::string_parsers::well_bracket_wrapped(tstr, '{', '}')) {
    throw fhicl::string_parsers::parser_fail()
        << "[ERROR]: Attempted to parse a non-map-like string ("
        << std::quoted(str) << ") as a ParameterSet.";

  } else {
    tstr = tstr.substr(1, tstr.size() - 2);
  }
  static const std::map<char, char> extra_brackets{
      {'[', ']'}, {'{', '}'}, {'(', ')'}};
  std::vector<std::string> k_v_list =
      fhicl::string_parsers::ParseToVect<std::string>(tstr, " ", false, true,
                                                      extra_brackets);
  if (k_v_list.size() % 2) {
    throw fhicl::string_parsers::parser_fail()
        << "[ERROR]: Attempted to parse a map-like string but expected an even "
           "number of values to be parsed as key: value pairs: "
        << std::quoted(tstr);
  }

  for (size_t i = 0; i < k_v_list.size(); i += 2) {

    if (k_v_list[i].back() != ':') {
      throw fhicl::string_parsers::parser_fail()
          << "[ERROR]: Attempted to parse a non-key-value-pair-like string ("
          << std::quoted(k_v_list[i] + " " + k_v_list[i + 1])
          << ") as a \"key: value\" pair.";
    }
    std::string const &k = k_v_list[i].substr(0, k_v_list[i].size() - 1);
    std::string const &v = k_v_list[i + 1];

    if (fhicl::string_parsers::is_table(v)) {
      internal_rep.insert({k, std::make_shared<ParameterSet>(v)});
    } else if (fhicl::string_parsers::is_sequence(v)) {
      internal_rep.insert({k, std::make_shared<Sequence>(v)});
    } else {
      internal_rep.insert({k, std::make_shared<Atom>(v)});
    }
  }
}

bool fhicl::ParameterSet::is_key_to_sequence(key_t const &key) const {
  if (!check_key(key)) {
    return false;
  }
  std::shared_ptr<fhicl::Sequence const> seq =
      std::dynamic_pointer_cast<fhicl::Sequence const>(
          get_value_recursive(key));
  return bool(seq);
}

std::string fhicl::ParameterSet::get_fhicl_category(
    std::shared_ptr<fhicl::Base const> el) const {
  if (!el) {
    return "nullptr";
  }
  std::shared_ptr<Atom const> atm = std::dynamic_pointer_cast<Atom const>(el);
  if (atm) {
    if (atm->is_nil()) {
      return "@nil";
    }
    return "atom";
  }
  std::shared_ptr<Sequence const> seq =
      std::dynamic_pointer_cast<Sequence const>(el);
  if (seq) {
    std::stringstream ss("");
    ss << "seq[" << seq->size() << "]";
    return ss.str();
  }
  std::shared_ptr<ParameterSet const> ps =
      std::dynamic_pointer_cast<ParameterSet const>(el);
  if (ps) {
    return "table";
  }
  throw bizare_error()
      << "[ERROR]: When attempting to get fhicl category, failed to cast as "
         "any known type. This is an internal error, please send a full "
         "backtrace to the maintainer.";
}

template <typename T>
typename std::enable_if<std::is_same<T, fhicl::ParameterSet>::value, void>::type
fhicl::ParameterSet::put_into_internal_rep(key_t const &key, T const &value) {
  get_value_recursive(key, true, true) =
      std::make_shared<typename fhicl::fhicl_type<T>::type>(value);
  idCache = 0;
}
template <typename T>
typename std::enable_if<!std::is_same<T, fhicl::ParameterSet>::value,
                        void>::type
fhicl::ParameterSet::put_into_internal_rep(key_t const &key, T const &value) {
  get_value_recursive(key, true, true) =
      std::make_shared<typename fhicl::fhicl_type<T>::type>(
          string_parsers::T2Str<T>(value));
  idCache = 0;
}

template <typename T>
void fhicl::ParameterSet::put_with_custom_history(key_t const &key,
                                                  T const &value,
                                                  std::string hist_entry) {
  std::stringstream ss("");
  if (has_key(key)) {
    ss << "Added a " << std::quoted(fhicl::fhicl_type<T>::type_string())
       << " from " << std::quoted(hist_entry);
  } else {
    ss << "Overriden with a "
       << std::quoted(fhicl::fhicl_type<T>::type_string()) << " from "
       << std::quoted(hist_entry);
  }

  put_into_internal_rep(key, value);
  history[key].push_back(ss.str());
}

// #define DEBUG_GET_VALUE_RECURSIVE

#ifdef DEBUG_GET_VALUE_RECURSIVE
static std::string indent = "";
#endif

std::shared_ptr<fhicl::Base> &fhicl::ParameterSet::get_value_recursive(
    fhicl::key_t const &key, bool allow_extend, bool allow_override) {
  static std::shared_ptr<Base> nullrtn(nullptr);
  nullrtn = nullptr;

#ifdef DEBUG_GET_VALUE_RECURSIVE
  std::cout << indent << "Looking for key: " << key << std::endl;
#endif

  size_t first_period = key.find_first_of(".");
  if (first_period == std::string::npos) { // No more recusion
#ifdef DEBUG_GET_VALUE_RECURSIVE
    std::cout << indent << "No periods, looking in this Parameter set."
              << std::endl;
#endif
    auto ki_pair = get_key_index(key);
#ifdef DEBUG_GET_VALUE_RECURSIVE
    std::cout << indent << "Split key to " << ki_pair.key << ", "
              << ki_pair.index << std::endl;
#endif
    auto kvp_it = internal_rep.find(ki_pair.key);
    if (kvp_it == internal_rep.end()) {
#ifdef DEBUG_GET_VALUE_RECURSIVE
      std::cout << indent << "No such key found." << std::endl;
#endif
      if (allow_extend) {
#ifdef DEBUG_GET_VALUE_RECURSIVE
        std::cout << indent << "Attempting to extend..." << std::endl;
#endif
        // attempt a sequence dereference
        if (ki_pair.has_index()) {
#ifdef DEBUG_GET_VALUE_RECURSIVE
          std::cout << indent << "Key has an index, making new sequence"
                    << std::endl;
#endif
          std::shared_ptr<fhicl::Sequence> seq =
              std::make_shared<fhicl::Sequence>();
#ifdef DEBUG_GET_VALUE_RECURSIVE
          std::cout << indent << "Adding new sequence to internal_rep"
                    << std::endl;
#endif
          internal_rep[ki_pair.key] = seq;
          added_key_for_extension(ki_pair.key, key);
#ifdef DEBUG_GET_VALUE_RECURSIVE
          std::cout << indent << "Extending child sequence." << std::endl;
#endif

#ifdef DEBUG_GET_VALUE_RECURSIVE
          std::cout << indent << "Before returning: " << to_string()
                    << std::endl;
#endif
          return seq->get_or_extend_get_value(ki_pair.index);
        } else {
#ifdef DEBUG_GET_VALUE_RECURSIVE
          std::cout << indent
                    << "Key has no index, adding new Atom to internal_rep"
                    << std::endl;
#endif
          internal_rep[ki_pair.key] = std::make_shared<Atom>();
          added_key_for_extension(ki_pair.key, key);
#ifdef DEBUG_GET_VALUE_RECURSIVE
          std::cout << indent << "Before returning: " << to_string()
                    << std::endl;
#endif
          return internal_rep[ki_pair.key];
        }
      } else {
#ifdef DEBUG_GET_VALUE_RECURSIVE
        std::cout << indent << "Not allowed to extend, return nullptr."
                  << std::endl;
#endif
        return nullrtn;
      }
    } else {
#ifdef DEBUG_GET_VALUE_RECURSIVE
      std::cout << indent << "Found the key" << std::endl;
#endif
      // attempt a sequence dereference
      if (ki_pair.has_index()) {
#ifdef DEBUG_GET_VALUE_RECURSIVE
        std::cout << indent << "Key has an index" << std::endl;
#endif
        if (!is_key_to_sequence(ki_pair.key)) {
          throw wrong_fhicl_category()
              << "[ERROR]: Attempted to access index of a sequence with key: "
              << std::quoted(key) << ", however the value is of fhicl type "
              << std::quoted(get_fhicl_category(key));
        }
        std::shared_ptr<fhicl::Sequence> seq =
            std::dynamic_pointer_cast<fhicl::Sequence>(
                internal_rep.at(ki_pair.key));
#ifdef DEBUG_GET_VALUE_RECURSIVE
        std::cout << indent
                  << "Get or extending sequence (Get: " << ki_pair.index
                  << ", Length: " << seq->size() << ")" << std::endl;
#endif
#ifdef DEBUG_GET_VALUE_RECURSIVE
        std::cout << indent << "Before returning: " << to_string() << std::endl;
#endif
        return seq->get_or_extend_get_value(ki_pair.index);
      } else {
#ifdef DEBUG_GET_VALUE_RECURSIVE
        std::cout << indent << "No index, returning entire object of type: "
                  << get_fhicl_category(kvp_it->first) << std::endl;
#endif
#ifdef DEBUG_GET_VALUE_RECURSIVE
        std::cout << indent << "Before returning: " << to_string() << std::endl;
#endif
        return kvp_it->second;
      }
    }
  } else { // we must go deepr
#ifdef DEBUG_GET_VALUE_RECURSIVE
    std::cout << indent
              << "Final element doesn't exist within this parameter set"
              << std::endl;
#endif
    std::string local_key = key.substr(0, first_period);
#ifdef DEBUG_GET_VALUE_RECURSIVE
    std::cout << indent << "Local portion of the key: " << local_key
              << std::endl;
#endif
    auto ki_pair = get_key_index(local_key);
#ifdef DEBUG_GET_VALUE_RECURSIVE
    std::cout << indent << "Split key to " << ki_pair.key << ", "
              << ki_pair.index << std::endl;
#endif

    auto kvp_it = internal_rep.find(ki_pair.key);
    if (kvp_it == internal_rep.end()) { // request key doesn't exist
#ifdef DEBUG_GET_VALUE_RECURSIVE
      std::cout << indent << "No such key found." << std::endl;
#endif
      if (allow_extend) { // we must make it
#ifdef DEBUG_GET_VALUE_RECURSIVE
        std::cout << indent << "Attempting to extend..." << std::endl;
#endif
        if (ki_pair.index !=
            std::numeric_limits<size_t>::max()) { // expecting a sequence
#ifdef DEBUG_GET_VALUE_RECURSIVE
          std::cout << indent << "Key has an index, making new sequence"
                    << std::endl;
#endif
          // make a new sequence
          std::shared_ptr<Sequence> seq = std::make_shared<Sequence>();
#ifdef DEBUG_GET_VALUE_RECURSIVE
          std::cout << indent << "Adding new sequence to internal_rep"
                    << std::endl;
#endif
          // put it in this parameter set
          internal_rep[ki_pair.key] = seq;
          added_key_for_extension(ki_pair.key, key);
#ifdef DEBUG_GET_VALUE_RECURSIVE
          std::cout << indent << "Making new ParameterSet" << std::endl;
#endif
          // make a new parameter set
          std::shared_ptr<ParameterSet> ps = std::make_shared<ParameterSet>();
#ifdef DEBUG_GET_VALUE_RECURSIVE
          std::cout << indent << "Adding ParameterSet to new child sequence"
                    << std::endl;
#endif
          // put it at the desired reference
          seq->get_or_extend_get_value(ki_pair.index) = ps;
#ifdef DEBUG_GET_VALUE_RECURSIVE
          std::cout << indent << "Before recursion: " << to_string()
                    << std::endl;

          std::cout << indent << "Recursing with sub-key: "
                    << key.substr(first_period + 1) << std::endl;
          indent = indent + "  ";
          // recurse into it
          std::shared_ptr<Base> &rtn = ps->get_value_recursive(
              key.substr(first_period + 1), allow_extend, allow_override);
          indent = indent.substr(2);
          return rtn;
#else
          // recurse into it
          return ps->get_value_recursive(key.substr(first_period + 1),
                                         allow_extend, allow_override);
#endif
        } else { // not expecting a sequence, just make an empty parameter set
// and recurse into it
#ifdef DEBUG_GET_VALUE_RECURSIVE
          std::cout << indent << "No index, making new ParameterSet"
                    << std::endl;
#endif
          std::shared_ptr<ParameterSet> ps = std::make_shared<ParameterSet>();
#ifdef DEBUG_GET_VALUE_RECURSIVE
          std::cout << indent << "Adding new ParameterSet to internal_rep"
                    << std::endl;
#endif
          internal_rep[ki_pair.key] = ps;
          added_key_for_extension(ki_pair.key, key);
#ifdef DEBUG_GET_VALUE_RECURSIVE
          std::cout << indent << "Before recursion: " << to_string()
                    << std::endl;
          std::cout << indent << "Recursing with sub-key: "
                    << key.substr(first_period + 1) << std::endl;
          indent = indent + "  ";
          // recurse into it
          std::shared_ptr<Base> &rtn = ps->get_value_recursive(
              key.substr(first_period + 1), allow_extend, allow_override);
          indent = indent.substr(2);
          return rtn;
#else
          // recurse into it
          return ps->get_value_recursive(key.substr(first_period + 1),
                                         allow_extend, allow_override);
#endif
        }
      } else { // too bad
#ifdef DEBUG_GET_VALUE_RECURSIVE
        std::cout << indent << "Not allowed to extend, return nullptr."
                  << std::endl;
#endif
        return nullrtn;
      }
    } else { // key exists
#ifdef DEBUG_GET_VALUE_RECURSIVE
      std::cout << indent << "Key exists." << std::endl;
#endif
      std::shared_ptr<Base> *child;
      // attempt a sequence dereference
      if (ki_pair.has_index()) {
#ifdef DEBUG_GET_VALUE_RECURSIVE
        std::cout << indent << "Found an index" << std::endl;
#endif
        if (!is_key_to_sequence(ki_pair.key)) {
          throw wrong_fhicl_category()
              << "[ERROR]: Attempted to access index of a sequence with key: "
              << std::quoted(key) << ", however the value is of fhicl type "
              << std::quoted(get_fhicl_category(key));
        }
        std::shared_ptr<fhicl::Sequence> seq =
            std::dynamic_pointer_cast<fhicl::Sequence>(
                internal_rep.at(ki_pair.key));
#ifdef DEBUG_GET_VALUE_RECURSIVE
        std::cout << indent
                  << "Get or extending sequence (Get: " << ki_pair.index
                  << ", Length: " << seq->size() << ")" << std::endl;
#endif
        child = &seq->get_or_extend_get_value(ki_pair.index);
      } else {
        child = &kvp_it->second;
      }

      std::shared_ptr<fhicl::ParameterSet> child_table =
          std::dynamic_pointer_cast<fhicl::ParameterSet>(*child);

      if (!child_table) {
        if (!allow_override) {
          throw wrong_fhicl_category()
              << "[ERROR]: Attempting to recurse into a "
                 "child parameter set named: "
              << std::quoted(local_key)
              << " from full key: " << std::quoted(key)
              << ", however the value is of fhicl type "
              << std::quoted(get_fhicl_category(key));
        } else {
#ifdef DEBUG_GET_VALUE_RECURSIVE
          std::cout << indent << "Retreived element is not a ParameterSet ("
                    << get_fhicl_category(child_table) << "), overriding"
                    << std::endl;
#endif
#ifdef DEBUG_GET_VALUE_RECURSIVE
          std::cout << indent << "No index, making new ParameterSet"
                    << std::endl;
#endif
          std::shared_ptr<ParameterSet> ps = std::make_shared<ParameterSet>();
#ifdef DEBUG_GET_VALUE_RECURSIVE
          std::cout << indent << "Adding new ParameterSet to internal_rep"
                    << std::endl;
#endif
          //
          (*child) = ps;
          overrode_key(ki_pair.key, ki_pair.index);
        }
      }
      child_table = std::dynamic_pointer_cast<fhicl::ParameterSet>(*child);
      if (!child_table) {
        throw bizare_error() << "[ERROR]: Failed to extend new table";
      }

#ifdef DEBUG_GET_VALUE_RECURSIVE
      std::cout << indent << "Before recursion: " << to_string() << std::endl;
      std::cout << indent
                << "Recursing with sub-key: " << key.substr(first_period + 1)
                << std::endl;
      indent = indent + "  ";
      // recurse into it
      std::shared_ptr<Base> &rtn = child_table->get_value_recursive(
          key.substr(first_period + 1), allow_extend, allow_override);
      indent = indent.substr(2);
      return rtn;
#else
      // recurse into it
      return child_table->get_value_recursive(key.substr(first_period + 1),
                                              allow_extend, allow_override);
#endif
    }
  }
}

std::shared_ptr<fhicl::Base> const &
fhicl::ParameterSet::get_value_recursive(fhicl::key_t const &key) const {
  static std::shared_ptr<Base> const nullrtn(nullptr);

#ifdef DEBUG_GET_VALUE_RECURSIVE
  std::cout << indent << "[cst] Looking for key: " << key << std::endl;
#endif

  size_t first_period = key.find_first_of(".");
  if (first_period == std::string::npos) { // No more recusion

#ifdef DEBUG_GET_VALUE_RECURSIVE
    std::cout << indent << "[cst] No periods, looking in this Parameter set."
              << std::endl;
#endif
    auto ki_pair = get_key_index(key);
#ifdef DEBUG_GET_VALUE_RECURSIVE
    std::cout << indent << "[cst] Split key to " << ki_pair.key << ", "
              << ki_pair.index << std::endl;
#endif
    auto kvp_it = internal_rep.find(ki_pair.key);
    if (kvp_it == internal_rep.end()) {
#ifdef DEBUG_GET_VALUE_RECURSIVE
      std::cout << indent
                << "[cst] Final element doesn't exist within this parameter set"
                << std::endl;
#endif
      return nullrtn;
    } else {
#ifdef DEBUG_GET_VALUE_RECURSIVE
      std::cout << indent << "[cst] Found the key" << std::endl;
#endif
      // attempt a sequence dereference
      if (ki_pair.has_index()) {
#ifdef DEBUG_GET_VALUE_RECURSIVE
        std::cout << indent << "[cst] Key has an index" << std::endl;
#endif
        if (!is_key_to_sequence(ki_pair.key)) {
          throw wrong_fhicl_category()
              << "[ERROR]: Attempted to access index of a sequence with key: "
              << std::quoted(key) << ", however the value is of fhicl type "
              << std::quoted(get_fhicl_category(key));
        }
        std::shared_ptr<fhicl::Sequence const> seq =
            std::dynamic_pointer_cast<fhicl::Sequence const>(
                internal_rep.at(ki_pair.key));
#ifdef DEBUG_GET_VALUE_RECURSIVE
        std::cout << indent << "[cst] Before returning: " << to_string()
                  << std::endl;
#endif
        return seq->get(ki_pair.index);
      } else {
#ifdef DEBUG_GET_VALUE_RECURSIVE
        std::cout << indent << "[cst] Before returning: " << to_string()
                  << std::endl;
#endif
        return kvp_it->second;
      }
    }
  } else { // we must go deepr
    std::string local_key = key.substr(0, first_period);
#ifdef DEBUG_GET_VALUE_RECURSIVE
    std::cout << indent << "[cst] Local portion of the key: " << local_key
              << std::endl;
#endif
    auto ki_pair = get_key_index(local_key);
#ifdef DEBUG_GET_VALUE_RECURSIVE
    std::cout << indent << "[cst] Split key to " << ki_pair.key << ", "
              << ki_pair.index << std::endl;
#endif
    auto kvp_it = internal_rep.find(ki_pair.key);
    if (kvp_it == internal_rep.end()) {
#ifdef DEBUG_GET_VALUE_RECURSIVE
      std::cout << indent << "[cst] No such key found." << std::endl;
      std::cout << indent << "[cst] Not allowed to extend, return nullptr."
                << std::endl;
#endif
      return nullrtn;
    } else {
#ifdef DEBUG_GET_VALUE_RECURSIVE
      std::cout << indent << "[cst] Key exists." << std::endl;
#endif

      std::shared_ptr<Base> const *child;
      // attempt a sequence dereference
      if (ki_pair.has_index()) {
        if (!is_key_to_sequence(ki_pair.key)) {
          throw wrong_fhicl_category()
              << "[ERROR]: Attempted to access index of a sequence with key: "
              << std::quoted(key) << ", however the value is of fhicl type "
              << std::quoted(get_fhicl_category(key));
        }
#ifdef DEBUG_GET_VALUE_RECURSIVE
        std::cout << indent << "[cst] Found an index" << std::endl;
#endif
        std::shared_ptr<fhicl::Sequence const> seq =
            std::dynamic_pointer_cast<fhicl::Sequence const>(
                internal_rep.at(ki_pair.key));
#ifdef DEBUG_GET_VALUE_RECURSIVE
        std::cout << indent
                  << "[cst] Get or extending sequence (Get: " << ki_pair.index
                  << ", Length: " << seq->size() << ")" << std::endl;
#endif
        child = &seq->get(ki_pair.index);
        // cannot extend in const method
        if (!(*child)) {
#ifdef DEBUG_GET_VALUE_RECURSIVE
          std::cout << indent << "[cst] Cannot extend in const method."
                    << std::endl;
#endif
          return nullrtn;
        }
      } else {
        child = &kvp_it->second;
      }

      std::shared_ptr<fhicl::ParameterSet const> const child_table =
          std::dynamic_pointer_cast<fhicl::ParameterSet const>(*child);

      if (!child_table) {
        throw wrong_fhicl_category()
            << "[ERROR]: Attempting to recurse into a "
               "child parameter set named: "
            << std::quoted(local_key) << " from full key: " << std::quoted(key)
            << ", however the value is of fhicl type "
            << std::quoted(get_fhicl_category(key));
      }

#ifdef DEBUG_GET_VALUE_RECURSIVE
      std::cout << indent << "[cst] Before recursion: " << to_string()
                << std::endl;
      std::cout << indent << "[cst] Recursing with sub-key: "
                << key.substr(first_period + 1) << std::endl;
      indent = indent + "  ";
      std::shared_ptr<Base> const &rtn =
          child_table->get_value_recursive(key.substr(first_period + 1));
      indent = indent.substr(2);
      return rtn;
#else
      return child_table->get_value_recursive(key.substr(first_period + 1));
#endif
    }
  }
}

#endif
