#pragma once

#include "fhiclcppsimple/ParameterSet.h"
#include "fhiclcppsimple/recursive_build_fhicl.hxx"

namespace fhiclsimple {

inline ParameterSet make_ParameterSet(std::string const &filename) {
  ParameterSet prolog;
  ParameterSet working_doc;

  fhiclsimple::fhicl_doc doc = fhiclsimple::read_doc(filename);
  doc.resolve_includes();

  return parse_fhicl_document(doc);
}

inline void make_ParameterSet(std::string const &filename, ParameterSet& pset){  
  pset = make_ParameterSet(filename);
}

} // namespace fhiclsimple
