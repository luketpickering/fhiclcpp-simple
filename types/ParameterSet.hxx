#ifndef FHICLCPP_SIMPLE_TYPES_PARAMETER_SET_HXX_SEEN
#define FHICLCPP_SIMPLE_TYPES_PARAMETER_SET_HXX_SEEN

namespace fhicl {
class ParameterSet : public Base {
  std::vector<std::shared_ptr<Base>> internal_rep;

public:
  template <typename T>
  ParameterSet(std::string const &str) : internal_rep() { from(str); }
  ParameterSet() : internal_rep() {}

  void from(std::string const &str) {
    for (std::string const &outer_list_item : str2T<std::vector<std::string>>(str, delim)) {
      if (is_table(outer_list_item)) {
        internal_rep.push_back(std::make_shared<ParameterSet>(outer_list_item));
      } else if (is_sequence(outer_list_item)) {
        internal_rep.push_back(std::make_shared<Sequence>(outer_list_item));
      } else {
        internal_rep.push_back(std::make_shared<Atom>(outer_list_item));
      }
    }
  }
};
} // namespace fhicl

#endif
