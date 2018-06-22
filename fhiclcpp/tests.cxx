#include <cassert>
#include <iostream>

#include "fhiclcpp/exception.hxx"
#include "fhiclcpp/make_ParameterSet.h"

using namespace fhicl;
using namespace linedoc;

#define operator_assert(L, OP, R)                                              \
  {                                                                            \
    if (!(L OP R)) {                                                           \
      std::cout << "ASSERT(" << #L << " " << #OP << " " << #R ") failed."      \
                << std::endl                                                   \
                << "  " << #L << " = \"" << L << "\"" << std::endl             \
                << "  " << #R << " = \"" << R << "\"" << std::endl;            \
    }                                                                          \
    assert((L OP R));                                                          \
  }

#define not_operator_assert(L, OP, R)                                          \
  {                                                                            \
    if ((L OP R)) {                                                            \
      std::cout << "ASSERT(" << #L << " " << #OP << " " << #R ") failed."      \
                << std::endl                                                   \
                << "  " << #L << " = \"" << L << "\"" << std::endl             \
                << "  " << #R << " = \"" << R << "\"" << std::endl;            \
    }                                                                          \
    assert(!(L OP R));                                                         \
  }

#define assert_are_equiv(doc, a, b)                                            \
  {                                                                            \
    if (!(doc.are_equivalent(a, b))) {                                         \
      std::cout << "ASSERT(" << #doc << ".are_equivalent(" << #a << ", " << #b \
                << ") failed on " << __FILE__ << ":" << __LINE__ << std::endl  \
                << "\t" << #a << " = \"" << a << "\"" << std::endl             \
                << "\t" << #b << " = \"" << b << "\"" << std::endl;            \
    }                                                                          \
    assert(doc.are_equivalent(a, b));                                          \
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
    doc_line_point match_1 =
        find_matching_bracket(test_doc, '{', '}', doc_line_point{0, 11});
    assert_are_equiv(test_doc, match_1, (doc_line_point{11, 0}));

    doc_line_point match_2 =
        find_matching_bracket(test_doc, '[', ']', doc_line_point{3, 6});
    assert_are_equiv(test_doc, match_2, (doc_line_point{3, 19}));
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
    doc_line_point begin{0, 0};
    doc_line_point eol{0, std::string::npos};
    doc_line_point first_line_just_off_end{0, 13};
    doc_line_point first_line_last_char{0, 12};
    doc_line_point mid_way_line_2{1, 9};

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

    doc_line_point fbracket{0, 11};

    doc_line_point match = find_matching_bracket(test_doc, '[', ']', fbracket);

    std::vector<doc_range> elements =
        get_list_elements(test_doc, {test_doc.advance(fbracket), match});

    std::vector<std::string> expected_results = {
        "a", "   b", "        b", " \",,{}[\'\'],\"", " [a,b,c]"};

    for (size_t i = 0; i < elements.size(); ++i) {
      operator_assert(expected_results[i], ==,
                      test_doc.substr(elements[i].begin, elements[i].end));
    }

    fhicl_doc test_doc2;
    test_doc2.push_back("[a,b,");
    test_doc2.push_back(" c ,  ,d");
    test_doc2.push_back(" e, f]");

    match = find_matching_bracket(test_doc2, '[', ']', doc_line_point::begin());
    elements = get_list_elements(
        test_doc2, {test_doc2.advance(doc_line_point::begin()), match}, true);

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
    } catch (unexpected_newline &e) {
      threw = true;
      std::cout << e.what() << std::endl;
    }
    if (!threw) {
      std::cout << ps.to_string() << std::endl;
    }
    assert(threw);

    threw = false;
    try {
      fhicl_doc doc = read_doc("fhiclcpp-simple.bad.newline.sequence.fcl");
      doc.resolve_includes();
      ps = parse_fhicl_document(doc);

    } catch (unexpected_newline &e) {
      threw = true;
      std::cout << e.what() << std::endl;
    }
    if (!threw) {
      std::cout << ps.to_string() << std::endl;
    }
    assert(threw);

    threw = false;
    try {
      fhicl_doc doc = read_doc("fhiclcpp-simple.bad.newline.string.fcl");
      doc.resolve_includes();
      ps = parse_fhicl_document(doc);

    } catch (unexpected_newline &e) {
      threw = true;
      std::cout << e.what() << std::endl;
    }
    if (!threw) {
      std::cout << ps.to_string() << std::endl;
    }
    assert(threw);
    std::cout << "[PASSED]: 3/3 bad newline tests" << std::endl;
  }
  {
    ParameterSet test_ps1;
    ParameterSet sub1;
    sub1.put("b", 5);
    sub1.put("c", 6);
    test_ps1.put("a", sub1);

    bool threw = false;
    ParameterSet ps;
    try {
      fhicl_doc doc = read_doc("fhiclcpp-simple.acceptable.newline.table.fcl");
      doc.resolve_includes();
      ps = parse_fhicl_document(doc);
    } catch (unexpected_newline &e) {
      threw = true;
      std::cout << e.what() << std::endl;
    }
    assert(!threw);
    operator_assert(test_ps1.id(), ==, ps.id());

    threw = false;
    ParameterSet test_ps2;
    test_ps2.put("d", std::vector<std::string>{"bla", "bla", "bla"});
    try {
      fhicl_doc doc =
          read_doc("fhiclcpp-simple.acceptable.newline.sequence.fcl");
      doc.resolve_includes();
      ps = parse_fhicl_document(doc);
    } catch (unexpected_newline &e) {
      threw = true;
      std::cout << e.what() << std::endl;
    }
    assert(!threw);
    operator_assert(test_ps2.id(), ==, ps.id());
    std::cout << "[PASSED]: 2/2 acceptable newline tests" << std::endl;
  }
}
