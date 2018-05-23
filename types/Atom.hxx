#ifndef FHICLCPP_SIMPLE_TYPES_ATOM_HXX_SEEN
#define FHICLCPP_SIMPLE_TYPES_ATOM_HXX_SEEN

#include "Base.hxx"

#include "string_parsers/from_string.hxx"
#include "string_parsers/traits.hxx"

#include <iomanip>
#include <sstream>

namespace fhicl {
class Atom : public Base {
public:
  template <typename T>
  typename std::enable_if<
      !is_seq<T>::value && !std::is_same<T, std::string>::value, T>::type
  as() {
    return string_parsers::str2T<T>(internal_rep);
  };
  template <typename T>
  typename std::enable_if<std::is_same<T, std::string>::value, T>::type as() {
    std::string stringified = string_parsers::str2T<T>(internal_rep);
    size_t first_punct = stringified.find_first_of(" ,\"\'");
    if (first_punct != std::string::npos) {
      std::stringstream ss("");
      ss << std::quoted(stringified);
      return ss.str();
    }
    return stringified;
  };
  Atom(std::string const &str) { from(str); }
  Atom() { internal_rep = "@nil"; }
  std::string as_string() { return as<std::string>(); }
  void from(std::string const &str) { internal_rep = str; }
};
} // namespace fhicl

#endif
