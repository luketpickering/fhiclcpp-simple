#include "parsers.hxx"

#include <iomanip>
#include <iostream>
#include <string>

int main(int argc, char const *argv[]) {
  if (argc != 2) {
    std::cout << "[ERROR]: Expected to be passed a .fcl file to pre-process."
              << std::endl;
  }

  std::cout << "[INFO]: Pre-processing file " << std::quoted(argv[1])
            << std::endl;

  auto fhicl_lines = fhicl::load_fhicl_file(argv[1]);

  auto fhicl = fhicl::fhiclpp(fhicl_lines);

  std::cout << std::quoted(fhicl) << std::endl;

  std::cout << "[INFO]: Test consume: " << std::endl;
  std::cout << std::quoted(fhicl::fhicl_consume(fhicl).to_indented_string())
            << std::endl;
}
