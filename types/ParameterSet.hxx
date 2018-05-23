#ifndef FHICLCPP_SIMPLE_TYPES_PARAMETER_SET_HXX_SEEN
#define FHICLCPP_SIMPLE_TYPES_PARAMETER_SET_HXX_SEEN

#include "Base.hxx"

#include "string_parsers/from_string.hxx"
#include "string_parsers/traits.hxx"

#include <memory>

namespace fhicl {

class Sequence;

class ParameterSet : public Base {
  std::map<std::string, std::shared_ptr<Base>> internal_rep;

public:
  template <typename T> T at_as(std::string const &key) {
    if (internal_rep.find(key) == internal_rep.end()) {
      throw;
    }
    return string_parsers::str2T<T>(internal_rep[key]->as_string());
  };
  ParameterSet get_ps(std::string const &key) {
    if (internal_rep.find(key) == internal_rep.end()) {
      throw;
    }
    std::shared_ptr<ParameterSet> ps =
        std::dynamic_pointer_cast<ParameterSet>(internal_rep[key]);
    if (!ps) {
      throw;
    }
    return *ps;
  }
  ParameterSet(std::string const &str) : internal_rep() { from(str); }
  ParameterSet() : internal_rep() {}

  std::string as_string() {
    std::stringstream ss("");
    ss << "{";
    for (auto ip_it = internal_rep.begin(); ip_it != internal_rep.end();
         ++ip_it) {
      ss << ip_it->first << ": " << ip_it->second->as_string() << " ";
    }
    ss << "}";
    return ss.str();
  }

  void from(std::string const &str);
};

} // namespace fhicl

#endif
