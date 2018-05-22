#ifndef FHICLSPP_SIMPLE_UTILS_HXX_SEEN
#define FHICLSPP_SIMPLE_UTILS_HXX_SEEN

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <locale>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace fhicl {

template <typename T> struct is_cont {
  static const bool value = false;
  static const bool not_value = true;
};

template <typename... Ts> struct is_cont<std::vector<Ts...>> {
  static const bool value = true;
  static const bool not_value = false;
};

// trim from start (in place)
static inline void ltrim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                                  [](int ch) { return !std::isspace(ch); }));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       [](int ch) { return !std::isspace(ch); })
              .base(),
          s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
  ltrim(s);
  rtrim(s);
}

template <typename T>
inline typename std::enable_if<is_cont<T>::not_value, T>::type
str2T(std::string const &str) {
  std::string cpy = str;
  trim(cpy);
  if (cpy == "@nil") {
    return T{};
  }
  std::istringstream stream(cpy);
  T d;
  stream >> d;

  if (stream.fail()) {
    std::cerr << "[WARN]: Failed to parse string: " << std::quoted(str)
              << " as requested type." << std::endl;
    throw;
    return T{};
  }

  return d;
}

template <> inline bool str2T<bool>(std::string const &str) {
  if ((str == "true") || (str == "True") || (str == "TRUE") || (str == "1")) {
    return true;
  }

  if ((str == "false") || (str == "False") || (str == "FALSE") ||
      (str == "0")) {
    return false;
  }

  std::istringstream stream(str);
  bool d;
  stream >> d;

  if (stream.fail()) {
    std::cerr << "[WARN]: Failed to parse string: " << str
              << " as requested type." << std::endl;
    return false;
  }

  return d;
}

template <typename T>
inline std::vector<T> ParseToVect(std::string const &inp,
                                  std::string const &delim,
                                  bool PushEmpty = false, bool trimInput = true,
                                  bool strip_brackets = false) {
  std::string inpCpy = inp;
  if (trimInput) {
    trim(inpCpy);
  }
  if (strip_brackets && ((inpCpy.front() == '[') || (inpCpy.back() == ']'))) {
    if (!((inpCpy.front() == '[') && (inpCpy.back() == ']'))) {
      std::cout << "[ERROR]: Tried to parse " << std::quoted(inp)
                << " into a vector delimited by " << std::quoted(delim)
                << ", and found mismatched square brackets." << std::endl;
      throw;
    }
    inpCpy = inpCpy.substr(1, inpCpy.size() - 2);
  }
  size_t nextOccurence = 0;
  size_t prevOccurence = 0;
  std::vector<T> outV;
  bool AtEnd = false;
  while (!AtEnd) {
    nextOccurence = inpCpy.find(delim, prevOccurence);
    if (nextOccurence == std::string::npos) {
      if (prevOccurence == inpCpy.length()) {
        break;
      }
      AtEnd = true;
    }
    if (PushEmpty || (nextOccurence != prevOccurence)) {
      outV.push_back(str2T<T>(
          inpCpy.substr(prevOccurence, (nextOccurence - prevOccurence))));
    }
    prevOccurence = nextOccurence + delim.size();
  }
  return outV;
}

template <typename T>
inline typename std::enable_if<is_cont<T>::value, T>::type
str2T(std::string const &str) {
  std::string tstr = str;
  trim(tstr);
  if (!tstr.size()) {
    std::cout << "[WARN]: Attempted to parse empty string as a vector of types."
              << std::endl;
    return T{};
  }
  if ((tstr[0] != '[') || (tstr[tstr.size() - 1] != ']')) {
    std::cout << "[WARN]: Attempted to parse a non-array-like string as a "
                 "vector of types."
              << std::endl;
    return T{};
  }

  return ParseToVect<typename T::value_type>(tstr, ",", true, true, true);
}

// #define FHICLCPP_SIMPLE_UTILS_DEBUG

#ifdef FHICLCPP_SIMPLE_UTILS_DEBUG
std::string indent = "";
void print_bracket_finder(std::string const &str, size_t begin = 0,
                          size_t end = 0) {
  std::cout << "[INFO]: " << indent << "\"" << str << "\"" << std::endl;
  std::cout << "         " << indent;
  for (size_t i = 0; i < begin; ++i) {
    std::cout << " ";
  }
  std::cout << "^";
  for (size_t i = begin + 1; i < end; ++i) {
    std::cout << " ";
  }
  std::cout << "^" << std::endl;
}
#endif

static std::map<char, char> matching_brackets = {
    {'(', ')'}, {'{', '}'}, {'[', ']'}, {'<', '>'}};
size_t find_matching_bracket(std::string const &str, char bracket = '{',
                             size_t begin = 0) {
  if (matching_brackets.find(bracket) == matching_brackets.end()) {
    std::cout << "[ERROR]: No matching bracket type known for \"" << bracket
              << "\"" << std::endl;
    throw;
  }
  if (str[begin] != bracket) {
    std::cout << "[ERROR]: Bad search starting position, expected to find \""
              << bracket << "\", but found \"" << str[begin] << "\""
              << std::endl;
    throw;
  }
  char match = matching_brackets.find(bracket)->second;

  size_t next_match = str.find(match, begin + 1);
  size_t next_open = str.find(bracket, begin + 1);
  while (next_open < next_match) {
#ifdef FHICLCPP_SIMPLE_UTILS_DEBUG
    indent += "  ";
#endif

    size_t next_open_match = find_matching_bracket(str, bracket, next_open);

#ifdef FHICLCPP_SIMPLE_UTILS_DEBUG
    if (indent.size()) {
      indent.erase(indent.size() - 2);
    }
#endif

    next_match = str.find(match, next_open_match + 1);
    next_open = str.find(bracket, next_open_match + 1);
  }

  if (next_match == std::string::npos) {
    std::cout << "[ERROR]: Failed to find matching bracket." << std::endl;
    throw;
  }
#ifdef FHICLCPP_SIMPLE_UTILS_DEBUG
  print_bracket_finder(str, begin, next_match);
#endif
  return next_match;
}

template <typename T>
inline std::string
T2Str(typename std::enable_if<is_cont<T>::not_value, T>::type const &o) {
  std::stringstream ss("");
  ss << o;
  return ss.str();
}
template <typename T>
inline std::string
T2Str(typename std::enable_if<is_cont<T>::value, T>::type const &vect) {
  std::stringstream ss("");

  ss << "[";
  for (size_t i = 0; i < vect.size(); ++i) {
    ss << T2Str<
              typename std::enable_if<is_cont<T>::value, T>::type::value_type>(
              vect[i])
       << (i + 1 == vect.size() ? "" : ",");
  }
  ss << "]";
  return ss.str();
}

template <> inline std::string T2Str<bool>(bool const &o) {
  return o ? "true" : "false";
}
template <> inline std::string T2Str<std::string>(std::string const &o) {
  return o;
}

} // namespace fhicl

#endif
