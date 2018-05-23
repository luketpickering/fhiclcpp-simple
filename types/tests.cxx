#include <cassert>
#include <iostream>

#include "Atom.hxx"
#include "ParameterSet.hxx"
#include "Sequence.hxx"

#include "CompositeTypesFromImpl.hxx"

using namespace fhicl;

int main(int argc, char const *argv[]) {
  {
    Atom a("5");
    assert((a.as<double>() == 5));
    Atom b("\"bla, bla\"");
    assert((b.as<std::string>() == "\"bla, bla\""));
    Atom filename("/path/to/file");
    assert((filename.as<std::string>() == "/path/to/file"));
    std::cout << "[PASSED] 3/3 Atom parse tests" << std::endl;
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
  {
    ParameterSet a(
        "{a: 5 b:  [5,6,7]    c: [a, \"bla, bla\", 5 ] d: {e: [5,5,5]}   }");
    std::cout << a.as_string() << std::endl;
    assert((a.at_as<double>("a") == 5));
    assert((a.at_as<std::array<double, 3>>("b") ==
            std::array<double, 3>{5, 6, 7}));
    assert((a.at_as<std::tuple<char, std::string, int>>("c") ==
            std::tuple<long, std::string, int>{'a', "bla, bla", 5}));
    ParameterSet d = a.get_ps("d");
    assert((d.at_as<std::vector<int>>("e") == std::vector<int>{5, 5, 5}));
    std::cout << "[PASSED] 4/4 ParameterSet parse tests" << std::endl;
  }
}
