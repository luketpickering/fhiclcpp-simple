#include <cassert>
#include <iostream>

#include "Atom.hxx"
#include "Sequence.hxx"

using namespace fhicl;

int main(int argc, char const *argv[]) {
  {
    Atom a("5");
    assert((a.as<double>() == 5));
    Atom b("\"bla, bla\"");
    assert((b.as<std::string>() == "bla, bla"));
    std::cout << "[PASSED] 2/2 Atom parse tests" << std::endl;
  }
  {
    Sequence a("[5]");
    assert((a.as<std::array<double, 1>>() == std::array<double, 1>{5}));
    Sequence b("[]");
    assert((b.as<std::array<double, 0>>() == std::array<double, 0>{}));
    Sequence c("[a,b]");
    assert((c.as<std::pair<std::string, std::string>>() ==
            std::pair<std::string, std::string>{"a", "b"}));
    Sequence d("[[a,b],[c]]");
    assert((d.as<std::vector<std::vector<std::string>>>() ==
            std::vector<std::vector<std::string>>{{"a", "b"}, {"c"}}));
    assert((d.at_as<std::vector<std::string>>(0) ==
            std::vector<std::string>{"a", "b"}));
    std::cout << "[PASSED] 5/5 Sequence parse tests" << std::endl;
  }
}
