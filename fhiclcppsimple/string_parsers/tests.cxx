#include <cassert>
#include <iostream>

#include "string_parsers/exception.hxx"
#include "string_parsers/from_string.hxx"
#include "string_parsers/to_string.hxx"
#include "string_parsers/utility.hxx"

using namespace fhiclsimple;
using namespace string_parsers;

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

int main() {
  {
    std::string table_like = " {a: 5 b: 6 }  ";
    std::string sequence_like = "[4,5,6,7]";
    std::string table_sequence = "{seq:[5,6,7]}";
    std::string sequence_table = "[4,{a:5 b:6}, @nil]";

    assert(is_table(table_like));
    assert(is_sequence(sequence_like));
    assert(is_table(table_sequence));
    assert(is_sequence(sequence_table));

    std::string malformed = "{ a:[5,6},7]";

    bool threw = false;
    try {
      is_table(malformed);
    } catch (mismatched_brackets &e) {
      assert(true);
      threw = true;
    }
    assert(threw);

    std::string sequence_like_odd_bracket = "[4,\"{\",6,7]";
    assert(is_sequence(sequence_like_odd_bracket));

    std::cout << "[PASSED] 6/6 Bracket matching and table/sequence string-type "
                 "deduction tests."
              << std::endl;
  }
  {
    int test_i = str2T<int>("5");
    operator_assert(test_i, ==, 5);

    int test_i_neg = str2T<int>("-5");
    operator_assert(test_i_neg, ==, -5);

    long test_l = str2T<long>("5555555555555");
    operator_assert(test_l, ==, 5555555555555l);

    std::cout << "[PASSED] 5/5 int parsing tests." << std::endl;

    bool test_true = str2T<bool>("true");
    assert((test_true == true));

    bool test_True = str2T<bool>("True");
    assert((test_True == true));

    bool test_false = str2T<bool>("false");
    assert((test_false == false));

    bool test_False = str2T<bool>("False");
    assert((test_False == false));

    std::cout << "[PASSED] 4/4 bool parsing tests." << std::endl;
  }
  {
    operator_assert(str2T<double>("-1E-5"), ==, double(-1E-5));

    operator_assert(str2T<double>("-123.456E-5"), ==, double(-123.456E-5));

    operator_assert(str2T<double>("-123.456E-1"), ==, double(-123.456E-1));

    double test_d_int = str2T<double>("5");
    assert((test_d_int == 5));

    double test_d_dec = str2T<double>("5.5");
    assert((test_d_dec == 5.5));

    double test_d_exp = str2T<double>("5E5");
    assert((test_d_exp == 5E5));

    double test_d_neg = str2T<double>("-5E5");
    assert((test_d_neg == -5E5));

    double test_d_exp_neg = str2T<double>("5E-5");
    assert((test_d_exp_neg == 5E-5));

    operator_assert(str2T<size_t>("1E5"), ==, size_t(1E5));
    operator_assert(str2T<int>("-1E5"), ==, int(-1E5));
    operator_assert(str2T<double>("-123.456E5"), ==, double(-123.456E5));
    operator_assert(str2T<double>("-123.456E1"), ==, double(-123.456E1));

    double test_d_hex = str2T<double>("0xdeadb33f");
    assert((test_d_hex == 0xdeadb33f));

    std::cout << "[PASSED] 10/10 double parsing tests." << std::endl;
  }
  {
    std::string test_str = str2T<std::string>("bla");
    assert((test_str == "bla"));

    std::string test_str_w_space = str2T<std::string>("bla ");
    assert((test_str_w_space == "bla"));

    std::string test_str_w_quotes = str2T<std::string>("\"bla \"");
    assert((test_str_w_quotes == "bla "));

    std::string test_str_w_odd_bracket = str2T<std::string>("\"bla{ \"");
    assert((test_str_w_odd_bracket == "bla{ "));

    std::cout << "[PASSED] 4/4 string parsing tests." << std::endl;
  }
  {
    bool trait_v = is_vect<std::vector<double>>::value;
    trait_v = trait_v && is_seq<std::vector<double>>::value;
    assert(trait_v);
    bool trait_a = is_array<std::array<double, 2>>::value;
    trait_a = trait_a && is_seq<std::array<double, 2>>::value;
    assert(trait_a);
    bool trait_p = is_pair<std::pair<double, double>>::value;
    trait_p = trait_p && is_seq<std::pair<double, double>>::value;
    assert(trait_p);
    bool trait_tup = is_tuple<std::tuple<double, double, double>>::value;
    trait_tup = trait_tup && is_seq<std::tuple<double, double, double>>::value;
    assert(trait_tup);

    std::cout << "[PASSED] 4/4 trait tests." << std::endl;
  }
  {
    std::vector<std::string> v =
        ParseToVect<std::string>("a,b", ",", true, true);
    assert((v == std::vector<std::string>{"a", "b"}));
  }
  {
    std::vector<double> test_v_d = str2T<std::vector<double>>("[5, 5.5, 5E5]");
    assert((test_v_d == std::vector<double>({5, 5.5, 5E5})));

    std::vector<std::string> test_v_s = str2T<std::vector<std::string>>(
        "[5, \"bla bla\", bla, \"hey ()(,),,-|| bla\", hello]");
    assert((test_v_s ==
            std::vector<std::string>(
                {"5", "bla bla", "bla", "hey ()(,),,-|| bla", "hello"})));

    bool threw = false;
    try {
      std::vector<std::vector<std::string>> test_v_s_nest_fail =
          str2T<std::vector<std::vector<std::string>>>(
              "[[5], \"bla bla\", [bla]]");
    } catch (parser_fail &e) {
      assert(true);
      threw = true;
    }
    assert(threw);

    std::vector<std::vector<std::string>> test_v_nested =
        str2T<std::vector<std::vector<std::string>>>(
            "[[5], [\"bla, bla\"], [5,6,7]]");
    assert((test_v_nested == std::vector<std::vector<std::string>>(
                                 {{"5"}, {"bla, bla"}, {"5", "6", "7"}})));

    std::vector<std::vector<std::vector<double>>> test_v_double_nest =
        str2T<std::vector<std::vector<std::vector<double>>>>(
            "[[[1,2,3],[4,5,6]],[[8],[9]]]");
    assert((test_v_double_nest == std::vector<std::vector<std::vector<double>>>(
                                      {{{1, 2, 3}, {4, 5, 6}}, {{8}, {9}}})));

    auto test_str_w_odd_bracket = str2T<std::vector<std::string>>(
        "[\"bla{ \", \"{}\", \"}\",\"[\",\"]\"]");
    assert((test_str_w_odd_bracket ==
            std::vector<std::string>{"bla{ ", "{}", "}", "[", "]"}));

    std::cout << "[PASSED] 6/6 vector parsing tests." << std::endl;
  }
  {
    std::pair<double, std::string> test_p_d_s =
        str2T<std::pair<double, std::string>>("[5, 5.5]");
    assert((test_p_d_s == std::pair<double, std::string>{5, "5.5"}));

    std::pair<double, std::pair<std::string, std::string>> test_p_d_p_s_s =
        str2T<std::pair<double, std::pair<std::string, std::string>>>(
            "[5, [5.5,\"bla, bla\"]]");
    assert((test_p_d_p_s_s ==
            std::pair<double, std::pair<std::string, std::string>>{
                5, {"5.5", "bla, bla"}}));

    std::pair<double, std::vector<bool>> test_p_d_v_b =
        str2T<std::pair<double, std::vector<bool>>>("[5, [true,true,false]]");
    assert((test_p_d_v_b ==
            std::pair<double, std::vector<bool>>{5, {true, true, false}}));

    std::cout << "[PASSED] 3/3 pair parsing tests." << std::endl;
  }
  {
    std::array<double, 2> test_a_d = str2T<std::array<double, 2>>("[5, 5.5]");
    assert((test_a_d == std::array<double, 2>{5, 5.5}));
    std::cout << "[PASSED] 1/1 array parsing tests." << std::endl;
  }
  {
    std::tuple<double, std::string, std::vector<std::string>> test_tup =
        str2T<std::tuple<double, std::string, std::vector<std::string>>>(
            "[5, 5.5, [\"bla, bla\",5, hello]]");
    assert(
        (test_tup == std::tuple<double, std::string, std::vector<std::string>>{
                         5, "5.5", {"bla, bla", "5", "hello"}}));

    std::vector<std::tuple<std::pair<std::string, std::string>, double>>
        test_vect_tup = str2T<std::vector<
            std::tuple<std::pair<std::string, std::string>, double>>>(
            "[[[hello,\"hello, hello\"],5],[[a,b],6]]");
    assert(
        (test_vect_tup ==
         std::vector<std::tuple<std::pair<std::string, std::string>, double>>{
             std::tuple<std::pair<std::string, std::string>, double>{
                 std::pair<std::string, std::string>{"hello", "hello, hello"},
                 5},
             std::tuple<std::pair<std::string, std::string>, double>{
                 std::pair<std::string, std::string>{"a", "b"}, 6}}));

    std::tuple<std::string, std::string, std::string> test_tup_s =
        str2T<std::tuple<std::string, std::string, std::string>>(
            "[\"bla, bla\",bla, bla]");
    assert((test_tup_s == std::tuple<std::string, std::string, std::string>{
                              "bla, bla", "bla", "bla"}));
    std::cout << "[PASSED] 3/3 tuple parsing tests." << std::endl;
  }
  {
    std::vector<std::tuple<std::pair<std::string, std::string>, double>>
        test_v_t_p{
            std::tuple<std::pair<std::string, std::string>, double>{
                std::pair<std::string, std::string>{"hello", "hello, hello"},
                5},
            std::tuple<std::pair<std::string, std::string>, double>{
                std::pair<std::string, std::string>{"a", "b"}, 6}};
    assert((T2Str<std::vector<
                std::tuple<std::pair<std::string, std::string>, double>>>(
                test_v_t_p) ==
            "[[[\"hello\",\"hello, hello\"],5],[[\"a\",\"b\"],6]]"));
    std::cout << "[PASSED] 1/1 to_string tests." << std::endl;
  }
}
