#ifndef FHICLCPP_SIMPLE_TYPES_SEQUENCE_HXX_SEEN
#define FHICLCPP_SIMPLE_TYPES_SEQUENCE_HXX_SEEN

#include "types/Base.hxx"

#include "string_parsers/from_string.hxx"
#include "string_parsers/traits.hxx"

#include <memory>

namespace fhicl {
class Sequence : public Base {
  std::vector<std::shared_ptr<Base>> internal_rep;

  void from(std::string const &str);

public:
  template <typename T> T at_as(size_t index) const {
    if (internal_rep.size() <= index) {
      throw;
    }
    return string_parsers::str2T<T>(internal_rep[index]->to_string());
  };
  template <typename T>
  typename std::enable_if<is_seq<T>::value, T>::type as() const {
    return string_parsers::str2T<T>(to_string());
  };
  Sequence(std::string const &str) : internal_rep() { from(str); }
  Sequence() : internal_rep() {}

  std::string to_string() const {
    std::stringstream ss("");
    ss << "[";
    for (size_t i = 0; i < internal_rep.size(); ++i) {
      ss << internal_rep[i]->to_string()
         << ((i + 1 == internal_rep.size()) ? "" : ", ");
    }
    ss << "]";
    return ss.str();
  }
  std::string to_compact_string() const {
    std::stringstream ss("");
    ss << "[";
    for (size_t i = 0; i < internal_rep.size(); ++i) {
      ss << internal_rep[i]->to_string()
         << ((i + 1 == internal_rep.size()) ? "" : ",");
    }
    ss << "]";
    return ss.str();
  }
  std::string to_indented_string(size_t indent_level) const {
    std::stringstream ss("");
    ss << "[" << std::endl;
    for (size_t i = 0; i < internal_rep.size(); ++i) {
      for (size_t i_it = 0; i_it < indent_level + 2; ++i_it) {
        ss << " ";
      }
      ss << internal_rep[i]->to_string()
         << ((i + 1 == internal_rep.size()) ? "" : ", ") << std::endl;
    }
    for (size_t i_it = 0; i_it < indent_level; ++i_it) {
      ss << " ";
    }
    ss << "]" << std::endl;
    return ss.str();
  }
};
} // namespace fhicl

#endif
