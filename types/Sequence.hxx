#ifndef FHICLCPP_SIMPLE_TYPES_SEQUENCE_HXX_SEEN
#define FHICLCPP_SIMPLE_TYPES_SEQUENCE_HXX_SEEN

#include "types/Base.hxx"

#include "string_parsers/from_string.hxx"
#include "string_parsers/traits.hxx"

#include <memory>

namespace fhicl {
class Sequence : public Base {
  std::vector<std::shared_ptr<Base>> internal_rep;

  void from(std::string const &str);

public:
  std::shared_ptr<fhicl::Base> &get_or_extend_get_value(size_t idx) {
    if (idx >= internal_rep.size()) {
      while (idx >= internal_rep.size()) {
        internal_rep.push_back(std::make_shared<Atom>());
      }
    }
    return internal_rep[idx];
  }
  std::shared_ptr<fhicl::Base> &get(size_t idx) {
    static std::shared_ptr<fhicl::Base> nullrtn(nullptr);
    // enusre that any busy_body who altered it by accident get flattened;
    nullrtn = nullptr;

    if (idx >= internal_rep.size()) {
      return nullrtn;
    }
    return internal_rep[idx];
  }
  std::shared_ptr<fhicl::Base> const &get(size_t idx) const {
    static std::shared_ptr<fhicl::Base> const nullrtn(nullptr);
    if (idx >= internal_rep.size()) {
      return nullrtn;
    }
    return internal_rep[idx];
  }

  size_t size() const { return internal_rep.size(); }

  template <typename T> T at_as(size_t index) const {
    if (internal_rep.size() <= index) {
      throw;
    }
    return string_parsers::str2T<T>(internal_rep[index]->to_string());
  };
  template <typename T>
  typename std::enable_if<is_seq<T>::value, T>::type as() const {
    return string_parsers::str2T<T>(to_string());
  };
  Sequence(std::string const &str) : internal_rep() { from(str); }
  Sequence() : internal_rep() {}

  std::string to_string() const {
    std::stringstream ss("");
    ss << "[";
    for (size_t i = 0; i < internal_rep.size(); ++i) {
      std::shared_ptr<ParameterSet> ps =
          std::dynamic_pointer_cast<ParameterSet>(internal_rep[i]);
      if (ps) {
        ss << "{ " << internal_rep[i]->to_string() << "} "
           << ((i + 1 == internal_rep.size()) ? "" : ", ");
      } else {
        ss << internal_rep[i]->to_string()
           << ((i + 1 == internal_rep.size()) ? "" : ", ");
      }
    }
    ss << "]";
    return ss.str();
  }
  std::string to_compact_string() const {
    std::stringstream ss("");
    ss << "[";
    for (size_t i = 0; i < internal_rep.size(); ++i) {
      std::shared_ptr<ParameterSet> ps =
          std::dynamic_pointer_cast<ParameterSet>(internal_rep[i]);
      if (ps) {
        ss << "{ " << internal_rep[i]->to_compact_string() << "}"
           << ((i + 1 == internal_rep.size()) ? "" : ",");
      } else {
        ss << internal_rep[i]->to_compact_string()
           << ((i + 1 == internal_rep.size()) ? "" : ",");
      }
    }
    ss << "]";
    return ss.str();
  }
  std::string to_indented_string(size_t indent_level) const {
    std::stringstream ss("");
    ss << "[" << std::endl;
    for (size_t i = 0; i < internal_rep.size(); ++i) {
      for (size_t i_it = 0; i_it < indent_level + 2; ++i_it) {
        ss << " ";
      }
      std::shared_ptr<ParameterSet> ps =
          std::dynamic_pointer_cast<ParameterSet>(internal_rep[i]);
      if (ps) {
        ss << "{ " << std::endl
           << internal_rep[i]->to_indented_string(indent_level + 4)
           << std::endl;
        for (size_t i_it = 0; i_it < (indent_level + 2); ++i_it) {
          ss << " ";
        }
        ss << "}" << ((i + 1 == internal_rep.size()) ? "" : ",");
      } else {
        ss << internal_rep[i]->to_indented_string(indent_level + 2)
           << ((i + 1 == internal_rep.size()) ? "" : ",");
      }
      ss << std::endl;
    }
    for (size_t i_it = 0; i_it < indent_level; ++i_it) {
      ss << " ";
    }
    ss << "]";
    return ss.str();
  }
  std::string to_indented_string_with_src_info(size_t indent_level) const {
    std::stringstream ss("");
    ss << "[" << std::endl;
    for (size_t i = 0; i < internal_rep.size(); ++i) {
      for (size_t i_it = 0; i_it < indent_level + 2; ++i_it) {
        ss << " ";
      }
      std::shared_ptr<ParameterSet> ps =
          std::dynamic_pointer_cast<ParameterSet>(internal_rep[i]);
      if (ps) {
        ss << "{ " << std::endl
           << internal_rep[i]->to_indented_string_with_src_info(indent_level +
                                                                4)
           << std::endl;
        for (size_t i_it = 0; i_it < (indent_level + 2); ++i_it) {
          ss << " ";
        }
        ss << "}"
           << "-- <NOINFO>"
           << ((i + 1 == internal_rep.size()) ? "" : ",");
      } else {
        ss << internal_rep[i]->to_indented_string_with_src_info(indent_level +
                                                                2)
           << ((i + 1 == internal_rep.size()) ? "" : ",");
      }
      ss << std::endl;
    }
    for (size_t i_it = 0; i_it < indent_level; ++i_it) {
      ss << " ";
    }
    ss << "]";
    return ss.str();
  }
};
} // namespace fhicl

#endif
