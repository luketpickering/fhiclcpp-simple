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
    if (internal_rep.size() <= index) {
      throw;
    }
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

  void from(std::string const &str);
};
} // namespace fhicl

#endif
