#include "utils.hxx"

#include <cassert>

using namespace fhicl;

int main() {
  double test_d_int = str2T<double>("5");
  assert((test_d_int == 5));

  double test_d_dec = str2T<double>("5.5");
  assert((test_d_dec == 5.5));

  double test_d_exp = str2T<double>("5E5");
  assert((test_d_exp == 5E5));

  std::vector<double> test_v_d = str2T<std::vector<double>>("[5, 5.5, 5E5]");
  assert((test_v_d == std::vector<double>({5, 5.5, 5E5})));

  std::string bracket_test = "{}{()[{}]{}}{}";
  size_t first_match = find_matching_bracket(bracket_test, '{');
  std::cout << "[INFO]: Found matching bracket at character: " << first_match
            << " = " << bracket_test[first_match] << std::endl;
  assert(first_match == 1);
  size_t level_0_match = find_matching_bracket(bracket_test, '{', 2);
  std::cout << "[INFO]: Found matching bracket at character: " << level_0_match
            << " = " << bracket_test[level_0_match] << std::endl;
  assert(level_0_match == 11);
  size_t level_1_match = find_matching_bracket(bracket_test, '(', 3);
  std::cout << "[INFO]: Found matching bracket at character: " << level_1_match
            << " = " << bracket_test[level_1_match] << std::endl;
  assert(level_1_match == 4);
  size_t level_2_match = find_matching_bracket(bracket_test, '[', 5);
  std::cout << "[INFO]: Found matching bracket at character: " << level_2_match
            << " = " << bracket_test[level_2_match] << std::endl;
  assert(level_2_match == 8);
}
