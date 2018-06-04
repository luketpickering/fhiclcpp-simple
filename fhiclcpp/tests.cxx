#include <cassert>
#include <iostream>

#include "fhiclcpp/exception.hxx"
#include "fhiclcpp/make_ParameterSet.h"

using namespace fhicl;

#define operator_assert(L, OP, R)                                              \
  {                                                                            \
    if (!(L OP R)) {                                                           \
      std::cout << "ASSERT(" << #L << " " << #OP << " " << #R ") failed."      \
                << std::endl                                                   \
                << "  " << #L << " = " << L << std::endl                       \
                << "  " << #R << " = " << R << std::endl;                      \
    }                                                                          \
    assert((L OP R));                                                          \
  }

#define not_operator_assert(L, OP, R)                                          \
  {                                                                            \
    if ((L OP R)) {                                                            \
      std::cout << "ASSERT(" << #L << " " << #OP << " " << #R ") failed."      \
                << std::endl                                                   \
                << "  " << #L << " = " << L << std::endl                       \
                << "  " << #R << " = " << R << std::endl;                      \
    }                                                                          \
    assert(!(L OP R));                                                         \
  }

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
    end = fhicl_doc_line_point::end();
    fhicl_doc_line_point eol{0, std::string::npos};

    assert(!begin.isend());
    assert(!begin.isendofline());

    assert(!eol.isend());
    assert(eol.isendofline());

    assert(end.isend());
    assert(end.isendofline());

    operator_assert(begin, ==, begin);
    operator_assert(eol, ==, eol);
    operator_assert(end, ==, end);

    operator_assert(begin, !=, eol);
    operator_assert(eol, !=, begin);
    operator_assert(eol, !=, end);
    operator_assert(end, !=, eol);

    not_operator_assert(begin, >, begin);
    not_operator_assert(eol, >, eol);
    not_operator_assert(end, >, end);

    not_operator_assert(begin, <, begin);
    not_operator_assert(eol, <, eol);
    not_operator_assert(end, <, end);

    operator_assert(begin, <, eol);
    operator_assert(eol, >, begin);

    not_operator_assert(begin, >, eol);
    not_operator_assert(eol, <, begin);

    operator_assert(begin, <, end);
    operator_assert(end, >, begin);

    not_operator_assert(begin, >, end);
    not_operator_assert(end, <, begin);

    operator_assert(eol, <, end);
    operator_assert(end, >, eol);

    not_operator_assert(eol, >, end);
    not_operator_assert(end, <, eol);

    std::vector<fhicl_doc_line_point> vect{end, eol, begin};
    std::sort(vect.begin(), vect.end());
    operator_assert(vect[0], ==, begin);
    operator_assert(vect[1], ==, eol);
    operator_assert(vect[2], ==, end);

    std::cout << "[PASSED]: 34/34 line_point comparison tests" << std::endl;
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

    std::cout << "[PASSED]: 6/6 validate_line_point tests" << std::endl;
  }
  {
    fhicl_doc_line_point begin{0, 0};
    fhicl_doc_line_point end_of_line{2, 3};
    fhicl_doc_line_point empty_line{6, 0};
    fhicl_doc_line_point last_char_doc{11, 1};

    fhicl_doc_line_point test = test_doc.advance(begin, 0);
    operator_assert(test_doc.get_char(test), ==, 's');

    test = test_doc.advance(begin, 1);
    operator_assert(test_doc.get_char(test), ==, 'o');

    test = test_doc.advance(begin, 5);
    operator_assert(test_doc.get_char(test), ==, 'h');

    test = test_doc.advance(end_of_line);
    assert(!test.isofftheend());
    operator_assert(test_doc.get_char(test), ==, 'e');

    test = test_doc.advance(empty_line, 0);
    assert(!test.isofftheend());
    operator_assert(test_doc.get_char(test), ==, ' ');

    test = test_doc.advance(empty_line, 1);
    assert(!test.isofftheend());
    operator_assert(test_doc.get_char(test), ==, 'c');

    test = test_doc.advance(begin, 1000);
    assert(test.isend());

    test = test_doc.advance(last_char_doc);
    assert(test.isend());

    std::cout << "[PASSED]: 11/11 fhicl_doc::advance tests" << std::endl;

    // test advance_line
  }
  {
    fhicl_doc test_doc;
    test_doc.push_back("");
    test_doc.push_back("abc");
    test_doc.push_back("def");
    test_doc.push_back("hij");
    test_doc.push_back("");

    fhicl_doc_line_point begin{0, 0};
    fhicl_doc_line_point eofl{0, 1};
    fhicl_doc_line_point sosl{1, 0};
    fhicl_doc_line_point end_of_line{1, 3};
    fhicl_doc_line_point l3{2, 1};
    fhicl_doc_line_point end = fhicl_doc_line_point::end();

    //{0,0} where the first line is empty is considered not the beginning
    begin = test_doc.validate_line_point(begin);
    operator_assert(begin, !=, fhicl_doc_line_point::begin());
    assert(begin.isendofline());

    eofl = test_doc.rewind(eofl, 0);
    operator_assert(eofl, ==, begin);
    eofl = test_doc.advance(eofl, 0);
    operator_assert(eofl, !=, begin);

    sosl = test_doc.rewind(sosl);
    operator_assert(sosl, ==, begin);

    fhicl_doc_line_point rew_0_end_of_line = test_doc.rewind(end_of_line, 0);
    operator_assert(test_doc.get_char(rew_0_end_of_line), ==, 'c');

    fhicl_doc_line_point rew_1_end_of_line = test_doc.rewind(end_of_line, 1);
    operator_assert(test_doc.get_char(rew_1_end_of_line), ==, 'b');

    fhicl_doc_line_point rew_2_end_of_line = test_doc.rewind(end_of_line, 2);
    operator_assert(test_doc.get_char(rew_2_end_of_line), ==, 'a');

    fhicl_doc_line_point rew_1_l3 = test_doc.rewind(l3, 1);
    operator_assert(test_doc.get_char(rew_1_l3), ==, 'd');

    fhicl_doc_line_point rew_2_l3 = test_doc.rewind(l3, 2);
    operator_assert(test_doc.get_char(rew_2_l3), ==, 'c');

    fhicl_doc_line_point rew_0_end = test_doc.rewind(end, 0);
    operator_assert(test_doc.get_char(rew_0_end), ==, 'j');

    fhicl_doc_line_point rew_1_end = test_doc.rewind(end, 1);
    operator_assert(test_doc.get_char(rew_1_end), ==, 'i');

    std::cout << "[PASSED]: 11/11 fhicl_doc::rewind tests" << std::endl;
  }
  {
    fhicl_doc test_doc;
    test_doc.push_back("");
    test_doc.push_back("abc");
    test_doc.push_back("def");
    test_doc.push_back("hij");
    test_doc.push_back("");
    operator_assert((test_doc.advance(fhicl_doc_line_point{1, 0}, 0)), ==,
                    (fhicl_doc_line_point{1, 0}));
    operator_assert((test_doc.rewind(fhicl_doc_line_point{1, 0}, 0)), ==,
                    (fhicl_doc_line_point{1, 0}));

    operator_assert((test_doc.advance(fhicl_doc_line_point{1, 1}, 0)), ==,
                    (fhicl_doc_line_point{1, 1}));
    operator_assert((test_doc.rewind(fhicl_doc_line_point{1, 1}, 0)), ==,
                    (fhicl_doc_line_point{1, 1}));

    operator_assert((test_doc.advance(fhicl_doc_line_point{1, 2}, 0)), ==,
                    (fhicl_doc_line_point{1, 2}));
    operator_assert((test_doc.rewind(fhicl_doc_line_point{1, 2}, 0)), ==,
                    (fhicl_doc_line_point{1, 2}));

    std::cout << "[PASSED]: 6/6 fhicl_doc::advance/rewind tests" << std::endl;
  }
  {
    fhicl_doc test_doc;
    test_doc.push_back("");
    test_doc.push_back("abc");
    test_doc.push_back("");
    test_doc.push_back("hij");
    test_doc.push_back("");
    assert(test_doc.are_equivalent(fhicl_doc_line_point{0, 0},
                                   fhicl_doc_line_point{1, 0}));
    assert(test_doc.are_equivalent(fhicl_doc_line_point{0, 0},
                                   fhicl_doc_line_point{0, 0}));
    assert(test_doc.are_equivalent(fhicl_doc_line_point{1, 3},
                                   fhicl_doc_line_point{2, 0}));
    assert(test_doc.are_equivalent(fhicl_doc_line_point{2, 0},
                                   fhicl_doc_line_point{1, 3}));
    assert(test_doc.are_equivalent(fhicl_doc_line_point{1, 3},
                                   fhicl_doc_line_point{3, 0}));
    assert(test_doc.are_equivalent(fhicl_doc_line_point{3, 0},
                                   fhicl_doc_line_point{1, 3}));

    std::cout << "[PASSED]: 6/6 equivalence tests" << std::endl;
  }
  {
    operator_assert(test_doc.find_first_of(':'), ==,
                    (fhicl_doc_line_point{0, 9}));
    operator_assert(test_doc.find_first_of('}'), ==,
                    (fhicl_doc_line_point{2, 0}));
    operator_assert(test_doc.find_first_of('v'), ==,
                    (fhicl_doc_line_point{3, 0}));
    operator_assert(test_doc.find_first_of('v', {3, 0}), ==,
                    (fhicl_doc_line_point{3, 0}));
    operator_assert(test_doc.find_first_of('v', {3, 1}), ==,
                    (fhicl_doc_line_point{4, 0}));
    assert(test_doc.find_first_of('@').isend());
    std::cout << "[PASSED]: 6/6 fhicl_doc::find_first_of tests" << std::endl;
  }
  {
    operator_assert(test_doc.find_first_not_of(' ', fhicl_doc_line_point{5, 2}),
                    ==, (fhicl_doc_line_point{8, 0}));
    operator_assert(
        test_doc.find_first_not_of('{', fhicl_doc_line_point{0, 10}), ==,
        (fhicl_doc_line_point{0, 10}));
    operator_assert(
        test_doc.find_first_not_of('{', fhicl_doc_line_point{0, 11}), ==,
        (fhicl_doc_line_point{0, 12}));
    std::cout << "[PASSED]: 3/3 fhicl_doc::find_first_not_of tests"
              << std::endl;
  }
  {
    fhicl_doc_line_point match_1 =
        find_matching_bracket(test_doc, '{', '}', fhicl_doc_line_point{0, 11});
    operator_assert(match_1, ==, (fhicl_doc_line_point{11, 0}));

    fhicl_doc_line_point match_2 =
        find_matching_bracket(test_doc, '[', ']', fhicl_doc_line_point{3, 6});
    operator_assert(match_2, ==, (fhicl_doc_line_point{3, 19}));
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
    operator_assert(test_substr, ==, "something: { ");

    test_substr = test_doc.substr(begin, first_line_just_off_end);
    operator_assert(test_substr, ==, "something: { ");

    test_substr = test_doc.substr(begin, first_line_last_char);
    operator_assert(test_substr, ==, "something: {");

    test_substr = test_doc.substr(begin, mid_way_line_2);
    operator_assert(test_substr, ==, "something: {  something");
    std::cout << "[PASSED]: 4/4 fhicl_doc substr tests" << std::endl;
  }
  {
    fhicl_doc test_doc;
    test_doc.push_back("Something: [a,   b,");
    test_doc.push_back("       b, \",,{}[\'\'],\", [a,b,c]]");

    fhicl_doc_line_point fbracket{0, 11};

    fhicl_doc_line_point match =
        find_matching_bracket(test_doc, '[', ']', fbracket);

    std::vector<fhicl_doc_range> elements =
        get_list_elements(test_doc, {test_doc.advance(fbracket), match});

    std::vector<std::string> expected_results = {
        "a", "   b", "       b", " \",,{}[\'\'],\"", " [a,b,c]"};

    for (size_t i = 0; i < elements.size(); ++i) {
      operator_assert(expected_results[i], ==,
                      test_doc.substr(elements[i].begin, elements[i].end));
    }

    fhicl_doc test_doc2;
    test_doc2.push_back("[a,b,");
    test_doc2.push_back(" c ,  ,d");
    test_doc2.push_back(" e, f]");

    match = find_matching_bracket(test_doc2, '[', ']',
                                  fhicl_doc_line_point::begin());
    elements = get_list_elements(
        test_doc2, {test_doc2.advance(fhicl_doc_line_point::begin()), match},
        true);

    expected_results = {
        "a", "b", "c", "",
        // extra whitespace for newline that substring adds a space for
        "d  e", "f"};

    for (size_t i = 0; i < elements.size(); ++i) {
      operator_assert(expected_results[i], ==,
                      test_doc2.substr(elements[i].begin, elements[i].end));
    }

    std::cout << "[PASSED]: 2/2 fhicl_doc get_list_elements tests" << std::endl;
  }
  {
    bool threw = false;
    ParameterSet ps;
    try {
      fhicl_doc doc = read_doc("fhiclcpp-simple.bad.newline.key.fcl");
      doc.resolve_includes();
      ps = parse_fhicl_document(doc);
    } catch (malformed_document &e) {
      threw = true;
      std::cout << e.what() << std::endl;
    }
    if (!threw) {
      std::cout << ps.to_string() << std::endl;
    }
    assert(threw);

    threw = false;
    // try {
    //   fhicl_doc doc = read_doc("fhiclcpp-simple.bad.newline.sequence.fcl");
    //   doc.resolve_includes();
    //   ps = parse_fhicl_document(doc);
    //
    // } catch (malformed_document &e) {
    //   threw = true;
    //   std::cout << e.what() << std::endl;
    // }
    // if (!threw) {
    //   std::cout << ps.to_string() << std::endl;
    // }
    // assert(threw);

    threw = false;
    try {
      fhicl_doc doc = read_doc("fhiclcpp-simple.bad.newline.string.fcl");
      doc.resolve_includes();
      ps = parse_fhicl_document(doc);

    } catch (malformed_document &e) {
      threw = true;
      std::cout << e.what() << std::endl;
    }
    if (!threw) {
      std::cout << ps.to_string() << std::endl;
    }
    assert(threw);
    std::cout << "[PASSED]: 3/3 bad newline tests" << std::endl;
  }
}
