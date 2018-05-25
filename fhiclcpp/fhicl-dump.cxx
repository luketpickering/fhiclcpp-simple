
#include "ParameterSet.h"
#include "make_ParameterSet.h"

#include <iostream>

int main(int argc, char const *argv[]) {
  if (argc != 2) {
    std::cout << "[ERROR]: Expected to be passed a single fcl file name."
              << std::endl;
    return 1;
  }

  fhicl::ParameterSet ps = fhicl::make_ParameterSet(argv[1]);
  std::cout << ps.to_indented_string_with_src_info() << std::endl;
}
