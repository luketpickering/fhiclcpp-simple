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
  ParameterSet a;

  {
    a.put("a", 5);
    a.put("b", std::vector<double>{5, 6, 7});
    a.put("c",
          std::tuple<std::string, std::string, double>{"a", "bla, bla", 5});
    ParameterSet d;
    d.put("e", std::array<double, 3>{5, 5, 5});
    a.put("d", d);
    std::cout << "[PASSED] 5/5 ParameterSet put tests" << std::endl;
  }
  {
    double a_a = a.get("a", double(0));
    assert(a_a == 5);

    double a_z = a.get("z", double(0xdeadb33f));
    assert(a_z == 0xdeadb33f);

    double a_a_ip = 0xdeadb33f;
    bool a_a_pres = a.get_if_present("a", a_a_ip);
    assert(a_a_pres);
    assert(a_a_ip == 5);

    double a_z_ip = 0xdeadb33f;
    bool a_z_pres = a.get_if_present("z", a_z_ip);
    assert(!a_z_pres);
    assert(a_z_ip == 0xdeadb33f);

    {
      bool threw = false;
      try {
        std::vector<double> v;
        v = a.get<std::vector<double>>("a");
        (void)v;
      } catch (wrong_fhicl_category &e) {
        threw = true;
      }
      assert(threw);
    }
    {
      bool threw = false;
      try {
        double b = a.get<double>("b");
        (void)b;
      } catch (wrong_fhicl_category &e) {
        threw = true;
      }
      assert(threw);
    }
    {
      bool threw = false;
      try {
        double d = a.get<double>("d");
        (void)d;
      } catch (wrong_fhicl_category &e) {
        threw = true;
      }
      assert(threw);
    }

    {
      bool threw = false;
      try {
        ParameterSet d = a.get<ParameterSet>("a");
        (void)d;
      } catch (wrong_fhicl_category &e) {
        threw = true;
      }
      assert(threw);
    }

    {
      bool threw = false;
      try {
        ParameterSet d = a.get<ParameterSet>("d");
        (void)d;
      } catch (wrong_fhicl_category &e) {
        threw = true;
      }
      assert(!threw);
    }

    std::cout << "[PASSED] 5/5 ParameterSet get tests" << std::endl;
  }
  ParameterSet ap(
      "{a: 5 b:  [5,6,7]    c: [a, \"bla, bla\", 5 ] d: {e: [5,5,5]}   }");
  {

    assert((ap.get<double>("a") == 5));
    assert(
        (ap.get<std::array<double, 3>>("b") == std::array<double, 3>{5, 6, 7}));
    assert((ap.get<std::tuple<char, std::string, int>>("c") ==
            std::tuple<long, std::string, int>{'a', "bla, bla", 5}));
    ParameterSet d = ap.get<ParameterSet>("d");
    assert((d.get<std::vector<int>>("e") == std::vector<int>{5, 5, 5}));
    std::cout << "[PASSED] 4/4 ParameterSet parse tests" << std::endl;
  }
  {
    assert((ap.id() == a.id()));
    std::cout << "[PASSED] 1/1 ParameterSet id tests" << std::endl;
  }
}
