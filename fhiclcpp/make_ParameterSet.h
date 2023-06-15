#pragma once

#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/recursive_build_fhicl.hxx"

namespace fhicl {

inline ParameterSet make_ParameterSet(std::string const &filename) {
  ParameterSet prolog;
  ParameterSet working_doc;

  fhicl::fhicl_doc doc = fhicl::read_doc(filename);
  doc.resolve_includes();

  return parse_fhicl_document(doc);
}

inline void make_ParameterSet(std::string const &filename, ParameterSet& pset){  
  pset = make_ParameterSet(filename);
}

} // namespace fhicl
