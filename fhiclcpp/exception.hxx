#ifndef FHICLCPP_SIMPLE_FHICLCPP_EXCEPTION_HXX_SEEN
#define FHICLCPP_SIMPLE_FHICLCPP_EXCEPTION_HXX_SEEN

#include "types/exception.hxx"

#include <sstream>
#include <stdexcept>

namespace fhicl {

#define NEW_EXCEPT(EXCEPT_NAME)                                                \
  struct EXCEPT_NAME : public fhicl_cpp_simple_except {                        \
    EXCEPT_NAME() : fhicl_cpp_simple_except() {}                               \
    EXCEPT_NAME(EXCEPT_NAME const &other) : fhicl_cpp_simple_except(other) {}  \
    template <typename T> EXCEPT_NAME &operator<<(T const &obj) {              \
      msgstrm << obj;                                                          \
      msg = msgstrm.str();                                                     \
      return (*this);                                                          \
    }                                                                          \
  }

NEW_EXCEPT(file_does_not_exist);
NEW_EXCEPT(include_loop);
NEW_EXCEPT(internal_error);
NEW_EXCEPT(malformed_document);

#undef NEW_EXCEPT

} // namespace fhicl

#endif
