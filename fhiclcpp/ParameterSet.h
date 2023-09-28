#pragma once

#include "fhiclcpp/types/ParameterSet.hxx"
#include "fhiclcpp/types/Sequence.hxx"

#include "fhiclcpp/types/CompositeTypesSharedImpl.hxx"

#include "fhiclcpp/recursive_build_fhicl.hxx"

namespace cet {
struct filepath_maker {};
} // namespace cet

namespace fhicl {

template <>
inline ParameterSet ParameterSet::make(std::string const &filename,
                                cet::filepath_maker &fm) {
  ParameterSet prolog;
  ParameterSet working_doc;

  fhicl::fhicl_doc doc = fhicl::read_doc(filename);
  doc.resolve_includes();

  return parse_fhicl_document(doc);
}

inline ParameterSet make_ParameterSet(std::string const &filename) {
  cet::filepath_maker dummy{};
  return ParameterSet::make(filename, dummy);
}

} // namespace fhicl
