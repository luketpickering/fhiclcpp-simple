#ifndef FHICLCPP_SIMPLE_TYPES_TRAITS_HXX_SEEN
#define FHICLCPP_SIMPLE_TYPES_TRAITS_HXX_SEEN

#include "types/Atom.hxx"
#include "types/ParameterSet.hxx"
#include "types/Sequence.hxx"

#include "string_parsers/traits.hxx"

namespace fhicl {

template <typename T> struct fhicl_type {
  static std::string const type_string() {
    return is_seq<T>::value ? "Sequence" : "Atom";
  }
  static constexpr bool is_table = false;
  typedef
      typename std::conditional<is_seq<T>::value, Sequence, Atom>::type type;
};

template <> struct fhicl_type<fhicl::ParameterSet> {
  static std::string const type_string() { return "ParameterSet"; }
  static constexpr bool is_table = true;
  typedef ParameterSet type;
};

} // namespace fhicl

#endif
