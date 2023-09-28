
#include "ParameterSet.h"

#include <iostream>

bool compact = false;

int main(int argc, char const *argv[]) {
  if ((argc != 2) && (argc != 3)) {
    std::cout << "[ERROR]: Expected to be passed an option -c compact "
                 "specifier and a single fcl file name."
              << std::endl;
    return 1;
  }

  std::string fname = argv[argc - 1];
  if ((argc == 3) && (std::string(argv[1]) == "-c")) {
    compact = true;
  }

  fhicl::ParameterSet ps = fhicl::make_ParameterSet(fname);

  std::cout << (compact ? ps.to_string() : ps.to_indented_string(2))
            << std::endl;
}
