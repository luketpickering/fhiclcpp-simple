#include <cassert>
#include <iostream>

#include "fhiclcppsimple/types/Atom.hxx"
#include "fhiclcppsimple/types/ParameterSet.hxx"
#include "fhiclcppsimple/types/Sequence.hxx"

#include "fhiclcppsimple/types/CompositeTypesSharedImpl.hxx"

using namespace fhicl;

int main() {
  {
    Atom a("5");
    assert((a.as<double>() == 5));
    Atom b("\"bla, bla\"");
    assert((b.as<std::string>() == "\"bla, bla\""));
    Atom filename("/path/to/file");
    assert((filename.as<std::string>() == "\"/path/to/file\""));
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
  {
    ParameterSet b;
    b.put("e.f.g[1].h", std::vector<double>{1, 2, 3});
    ParameterSet c("{ e: { f: { g: [@nil, { h: [1,2,3 ] } ] } } }");
    assert((b.id() == c.id()));
    std::cout << "[PASSED] 1/1 Automatic put extension tests" << std::endl;
    std::cout << b.to_indented_string_with_src_info() << std::endl;
    std::cout << b.to_indented_string() << std::endl;
    std::cout << b.history_to_string() << std::endl;
  }
  {
    ParameterSet c("{a:[{b:c d:e},{f:g h:i}]}");
    ParameterSet ps_0 = c.get<ParameterSet>("a[0]");
    assert(ps_0.get<char>("d") == 'e');
    std::vector<ParameterSet> ps_list = c.get<std::vector<ParameterSet>>("a");
    assert((ps_list[0].id() == ps_0.id()));
    std::cout << ps_list[0].to_indented_string() << std::endl;
    ParameterSet e =
        string_parsers::str2T<ParameterSet>("{a:[{b:c d:e},{f:g h:i}]}");
    assert((c.id() == e.id()));
    std::cout << "[PASSED] 3/3 get ParameterSet tests" << std::endl;
  }
  {
    ParameterSet b;
    b.put("a", std::vector<double>{1, 2, 3});
    ParameterSet c;
    c.put("c", b);
    ParameterSet e;
    e.put<std::pair<double, double>>("bla", {0.5, 1.5});
    ParameterSet d;
    d.put<std::vector<ParameterSet>>("d", {{c, b, c, b}});
    d.put<std::vector<ParameterSet>>("e", {{e, e, e}});

    std::cout << d.to_indented_string() << std::endl;
  }
}
