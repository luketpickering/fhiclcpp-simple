#include <cassert>
#include <iostream>

#include "fhiclcpp/exception.hxx"
#include "fhiclcpp/make_ParameterSet.h"

using namespace fhicl;

int main() {
  fhicl_doc test_doc;
  test_doc.push_back("something: { ");
  test_doc.push_back("something_else: { ");
  test_doc.push_back("} ");
  test_doc.push_back("vect: [a,s, \"{\",d,f] ");
  test_doc.push_back("vect2: [a,");
  test_doc.push_back("b,");
  test_doc.push_back("");
  test_doc.push_back(" ");
  test_doc.push_back("c,");
  test_doc.push_back("d,");
  test_doc.push_back("]");
  test_doc.push_back("} ");
  {
    fhicl_doc_line_point begin{0, 0};
    fhicl_doc_line_point end{0, 0};
    end.setend();
    fhicl_doc_line_point eol{0, std::string::npos};

    assert(!begin.isend());
    assert(end.isend());
    assert(eol.isendofline());
    assert(eol < end);
    eol.setend();
    assert((end == eol));
    assert((begin == begin));
    assert((begin < end));
    assert((begin < eol));
    std::cout << "[PASSED]: 8/8 line_point tests" << std::endl;
  }
  {
    fhicl_doc_line_point begin{0, 0};
    fhicl_doc_line_point last_char_line{2, 1};
    fhicl_doc_line_point end_of_line{2, 3};
    fhicl_doc_line_point empty_line{6, 0};

    begin = test_doc.validate_line_point(begin);
    last_char_line = test_doc.validate_line_point(last_char_line);
    end_of_line = test_doc.validate_line_point(end_of_line);
    empty_line = test_doc.validate_line_point(empty_line);

    assert(!begin.isofftheend());
    assert(!last_char_line.isofftheend());
    assert(end_of_line.isofftheend());
    assert(end_of_line.isofftheend() && !end_of_line.isend());
    assert(empty_line.isofftheend() && !empty_line.isend());

    std::cout << "[PASSED]: 5/5 validate_line_point tests" << std::endl;
  }
  {
    fhicl_doc_line_point begin{0, 0};
    fhicl_doc_line_point end_of_line{2, 3};
    fhicl_doc_line_point empty_line{6, 0};
    fhicl_doc_line_point last_char_doc{11, 1};

    fhicl_doc_line_point test = test_doc.advance(begin, 0);
    assert(test_doc.get_char(test) == 's');

    test = test_doc.advance(begin, 1);
    assert(test_doc.get_char(test) == 'o');

    test = test_doc.advance(begin, 5);
    assert(test_doc.get_char(test) == 'h');

    test = test_doc.advance(end_of_line);
    assert(!test.isofftheend());
    assert(test_doc.get_char(test) == 'e');

    test = test_doc.advance(empty_line, 0);
    assert(!test.isofftheend());
    assert(test_doc.get_char(test) == ' ');

    test = test_doc.advance(empty_line, 1);
    assert(!test.isofftheend());
    assert(test_doc.get_char(test) == 'c');

    test = test_doc.advance(begin, 1000);
    assert(test.isend());

    test = test_doc.advance(last_char_doc);
    assert(test.isend());

    std::cout << "[PASSED]: 11/11 fhicl_doc::advance tests" << std::endl;

    // test advance_line
  }
  {
    assert((test_doc.find_first_of(':') == fhicl_doc_line_point{0, 9}));
    assert((test_doc.find_first_of('}') == fhicl_doc_line_point{2, 0}));
    assert((test_doc.find_first_of('v') == fhicl_doc_line_point{3, 0}));
    assert((test_doc.find_first_of('v', {3, 0}) == fhicl_doc_line_point{3, 0}));
    assert((test_doc.find_first_of('v', {3, 1}) == fhicl_doc_line_point{4, 0}));
    assert((test_doc.find_first_of('@').isend()));
    std::cout << "[PASSED]: 6/6 fhicl_doc::find_first_of tests" << std::endl;
  }
  {
    assert((test_doc.find_first_not_of(' ', fhicl_doc_line_point{5, 2}) ==
            fhicl_doc_line_point{8, 0}));
    assert((test_doc.find_first_not_of('{', fhicl_doc_line_point{0, 10}) ==
            fhicl_doc_line_point{0, 10}));
    assert((test_doc.find_first_not_of('{', fhicl_doc_line_point{0, 11}) ==
            fhicl_doc_line_point{0, 12}));
    std::cout << "[PASSED]: 3/3 fhicl_doc::find_first_not_of tests"
              << std::endl;
  }
  {
    fhicl_doc_line_point match_1 =
        find_matching_bracket(test_doc, '{', '}', fhicl_doc_line_point{0, 11});
    assert((match_1 == fhicl_doc_line_point{11, 0}));

    fhicl_doc_line_point match_2 =
        find_matching_bracket(test_doc, '[', ']', fhicl_doc_line_point{3, 6});
    assert((match_2 == fhicl_doc_line_point{3, 19}));
    std::cout << "[PASSED]: 2/2 find_matching_bracket tests" << std::endl;
  }
  {
    bool threw = false;
    try {
      fhicl_doc doc = read_doc("fhiclcpp-simple.recursive.include1.fcl");
      doc.resolve_includes();
    } catch (include_loop) {
      threw = true;
    }
    assert(threw);
    std::cout << "[PASSED]: 1/1 include loop tests" << std::endl;
  }
  {
    fhicl_doc test_doc;
    test_doc.push_back("something: { ");
    test_doc.push_back("something_else: { ");
    test_doc.push_back("} ");
    test_doc.push_back("vect: [a,s, \"{\",d,f] ");
    test_doc.push_back("vect2: [a,");
    test_doc.push_back("b,");
    test_doc.push_back("");
    test_doc.push_back(" ");
    test_doc.push_back("c,");
    test_doc.push_back("d,");
    test_doc.push_back("]");
    test_doc.push_back("} ");

    fhicl_doc_line_point begin{0, 0};
    fhicl_doc_line_point eol{0, std::string::npos};
    fhicl_doc_line_point first_line_just_off_end{0, 13};
    fhicl_doc_line_point first_line_last_char{0, 12};
    fhicl_doc_line_point mid_way_line_2{1, 9};

    std::string test_substr = test_doc.substr(begin, eol);
    std::cout << std::quoted(test_substr) << std::endl;
    assert(test_substr == "something: { ");

    test_substr = test_doc.substr(begin, first_line_just_off_end);
    std::cout << std::quoted(test_substr) << std::endl;
    assert(test_substr == "something: { ");

    test_substr = test_doc.substr(begin, first_line_last_char);
    std::cout << std::quoted(test_substr) << std::endl;
    assert(test_substr == "something: {");

    test_substr = test_doc.substr(begin, mid_way_line_2);
    std::cout << std::quoted(test_substr) << std::endl;
    assert(test_substr == "something: {  something");
  }
  {
    // test subdoc
    // test subdoc doesn't include final char
  }
}
