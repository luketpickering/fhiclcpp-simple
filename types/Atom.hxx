#ifndef FHICLCPP_SIMPLE_TYPES_ATOM_HXX_SEEN
#define FHICLCPP_SIMPLE_TYPES_ATOM_HXX_SEEN

#include "Base.hxx"

#include "string_parsers/from_string.hxx"
#include "string_parsers/traits.hxx"

namespace fhicl {
class Atom : public Base {
public:
  template <typename T>
  typename std::enable_if<!is_seq<T>::value, T>::type as() {
    return string_parsers::str2T<T>(internal_rep);
  };
  Atom(std::string const &str) { from(str); }
  Atom() { internal_rep = "@nil"; }
  std::string as_string() { return as<std::string>(); }
  void from(std::string const &str) { internal_rep = str; }
};
} // namespace fhicl

#endif
