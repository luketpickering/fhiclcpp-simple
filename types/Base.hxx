#ifndef FHICLCPP_SIMPLE_TYPES_BASE_HXX_SEEN
#define FHICLCPP_SIMPLE_TYPES_BASE_HXX_SEEN

#include <string>

namespace fhicl {
class Base {
protected:
  std::string internal_rep;
  Base() : internal_rep() {}
  Base(std::string const &str) : internal_rep(str) {}
  Base(std::string &&str) : internal_rep(std::move(str)) {}

  virtual ~Base(){};

public:
  virtual std::string as_string() = 0;
  virtual void from(std::string const &) = 0;
};
} // namespace fhicl

#endif
