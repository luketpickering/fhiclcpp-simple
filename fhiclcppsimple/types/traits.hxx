#pragma once

#include "fhiclcppsimple/types/Atom.hxx"
#include "fhiclcppsimple/types/ParameterSet.hxx"
#include "fhiclcppsimple/types/Sequence.hxx"

#include "fhiclcppsimple/string_parsers/traits.hxx"

namespace fhiclsimple {

#define VARLIST                                                                \
  Y(kNil, 1)                                                                   \
  X(kAtom)                                                                     \
  X(kSequence)                                                                 \
  X(kTable)                                                                    \
  X(kInvalidInstance)

#define X(A) A,
#define Y(A, B) A = B,
enum class fhicl_category { VARLIST };
#undef X
#undef Y
#define X(A)                                                                   \
  case fhicl_category::A: {                                                    \
    return os << #A;                                                           \
  }
#define Y(A, B)                                                                \
  case fhicl_category::A: {                                                    \
    return os << #A;                                                           \
  }
inline std::ostream &operator<<(std::ostream &os, fhicl_category te) {
  switch (te) { VARLIST }
  return os;
}
#undef X
#undef Y
#undef VARLIST

template <typename T> struct fhicl_type {
  static fhicl_category category() {
    return is_seq<T>::value ? fhicl_category::kSequence : fhicl_category::kAtom;
  }
  static std::string const category_string() {
    std::stringstream ss("");
    ss << category();
    return ss.str();
  }
  static constexpr bool is_table = false;
  typedef
      typename std::conditional<is_seq<T>::value, Sequence, Atom>::type type;
};

template <> struct fhicl_type<fhiclsimple::ParameterSet> {
  static fhicl_category category() { return fhicl_category::kTable; }
  static std::string const category_string() { return "kTable"; }
  static constexpr bool is_table = true;
  typedef ParameterSet type;
};
template <> struct fhicl_type<fhiclsimple::Atom> {
  static fhicl_category category() { return fhicl_category::kAtom; }
  static std::string const category_string() { return "kAtom"; }
  static constexpr bool is_table = false;
  typedef Atom type;
};
template <> struct fhicl_type<fhiclsimple::Sequence> {
  static fhicl_category category() { return fhicl_category::kSequence; }
  static std::string const category_string() { return "kSequence"; }
  static constexpr bool is_table = false;
  typedef Sequence type;
};

} // namespace fhiclsimple
