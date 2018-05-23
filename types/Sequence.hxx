#ifndef FHICLCPP_SIMPLE_TYPES_SEQUENCE_HXX_SEEN
#define FHICLCPP_SIMPLE_TYPES_SEQUENCE_HXX_SEEN

#include "Base.hxx"

#include "string_parsers/from_string.hxx"
#include "string_parsers/traits.hxx"

#include <memory>

namespace fhicl {
class Sequence : public Base {
  std::vector<std::shared_ptr<Base>> internal_rep;

public:
  template <typename T> T at_as(size_t index) {
    return string_parsers::str2T<T>(internal_rep[index]->as_string());
  };
  template <typename T>
  typename std::enable_if<is_seq<T>::value, T>::type as() {
    return string_parsers::str2T<T>(as_string());
  };
  Sequence(std::string const &str) : internal_rep() { from(str); }
  Sequence() : internal_rep() {}

  std::string as_string() {
    std::stringstream ss("");
    ss << "[";
    for (size_t i = 0; i < internal_rep.size(); ++i) {
      ss << internal_rep[i]->as_string()
         << ((i + 1 == internal_rep.size()) ? "" : ",");
    }
    ss << "]";
    return ss.str();
  }

  void from(std::string const &str) {
    for (std::string const &outer_list_item :
         string_parsers::str2T<std::vector<std::string>>(str)) {
      if (string_parsers::is_table(outer_list_item)) {
        // internal_rep.push_back(std::make_shared<ParameterSet>(outer_list_item));
      } else if (string_parsers::is_sequence(outer_list_item)) {
        internal_rep.push_back(std::make_shared<Sequence>(outer_list_item));
      } else {
        internal_rep.push_back(std::make_shared<Atom>(outer_list_item));
      }
    }
  }
};
} // namespace fhicl

#endif
