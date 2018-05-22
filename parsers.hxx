#ifndef FHICLCPP_SIMPLE_PARSERS_HXX_SEEN
#define FHICLCPP_SIMPLE_PARSERS_HXX_SEEN

#include "types.hxx"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace fhicl {

typedef std::vector<std::string> fhicl_file_preproc_t;
typedef std::string fhicl_file_t;

fhicl_file_preproc_t load_fhicl_file(std::string const &filename) {
  // Search for file in fhicl_file_path
  // open file
  std::ifstream ifs(filename);
  if (!ifs.good()) {
    std::cerr << "[ERROR]: File \"" << filename
              << " could not be opened for reading." << std::endl;
    throw;
  }
  std::string line;
  fhicl_file_preproc_t fhicl_lines;
  while (std::getline(ifs, line)) {
    trim(line);
    fhicl_lines.push_back(line);
  }
  return fhicl_lines;
}

void fhicl_strip_comments(fhicl_file_preproc_t &fhicl_lines) {
  size_t NLines = fhicl_lines.size();
  for (size_t ln_it = 0; ln_it < NLines; ++ln_it) {
    size_t comment_begin = fhicl_lines[ln_it].find("//");
    if (comment_begin != std::string::npos) {
      fhicl_lines[ln_it].erase(comment_begin);
    }
    comment_begin = fhicl_lines[ln_it].find("#");
    if (comment_begin != std::string::npos) {
      fhicl_lines[ln_it].erase(comment_begin);
    }
    trim(fhicl_lines[ln_it]);
  }
}
void fhicl_resolve_include_statements(fhicl_file_preproc_t &fhicl_lines) {

  size_t n_replacements;
  do {
    n_replacements = 0;

    size_t NLines = fhicl_lines.size();
    for (size_t ln_it = 0; ln_it < NLines; ++ln_it) {
      if (fhicl_lines[ln_it].find("#include") == 0) {
        std::vector<std::string> splits =
            ParseToVect<std::string>(fhicl_lines[ln_it], "\"");
        if ((splits.size() > 2) || (fhicl_lines[ln_it].back() != '\"')) {
          std::cout << "[ERROR]: Found malformed include line: \'"
                    << fhicl_lines[ln_it]
                    << "\', expected to find: \'#include \"external.fcl\"\'."
                    << std::endl;
          throw;
        }
        fhicl_file_preproc_t external = load_fhicl_file(splits.back());

        fhicl_file_preproc_t::iterator inc_it =
            fhicl_lines.erase(fhicl_lines.begin() + ln_it);
        fhicl_lines.insert(external.begin(), external.end(), inc_it);

        n_replacements++;
      }
    }

  } while (n_replacements);
}
fhicl_file_t fhiclpp(fhicl_file_preproc_t fhicl_lines) {
  fhicl_resolve_include_statements(fhicl_lines);
  fhicl_strip_comments(fhicl_lines);
  std::stringstream ss("");
  size_t NLines = fhicl_lines.size();
  for (size_t ln_it = 0; ln_it < NLines; ++ln_it) {
    if (!fhicl_lines[ln_it].size()) {
      continue;
    }
    ss << fhicl_lines[ln_it] << (((ln_it + 1) == NLines) ? "" : " ");
  }

  return ss.str();
}

#define FHICLCPP_SIMPLE_PARSERS_DEBUG

ParameterSet fhicl_consume(fhicl_file_t &fhicl) {
  bool InProlog = false;
  ParameterSet ps;

  size_t read_ptr = fhicl.find_first_not_of(" ", 0);
  while (read_ptr != std::string::npos) {
    size_t next_char = fhicl.find_first_of(" ", read_ptr);
    std::string token = fhicl.substr(read_ptr, next_char - read_ptr);

#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
    std::cout << "Reading: (" << read_ptr << ": " << next_char
              << ") = " << token << std::endl;
#endif

    if (token == "BEGIN_PROLOG") {
      InProlog = true;
#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
      std::cout << "[INFO]: In prolog." << std::endl;
#endif
    } else if (token == "END_PROLOG") {
      InProlog = false;
#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
      std::cout << "[INFO]: No longer in prolog." << std::endl;
#endif
    } else {
      if (token.back() != ':') {
        std::cout << "[ERROR]: Expected a key declaraction like \"key:\", but "
                     "instead found \""
                  << token << "\"" << std::endl;
        throw;
      }
      std::string key = token.substr(0, token.size() - 1);

      // respond based on value type:
      size_t next_not_space = fhicl.find_first_not_of(" ", next_char + 1);
      switch (fhicl[next_not_space]) {
      case '{': {
        // table member
        next_char = find_matching_bracket(fhicl, '{', next_not_space);
        std::string value =
            fhicl.substr(next_not_space + 1, next_char - (next_not_space + 1));
#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
        std::cout << "[INFO]: Found table KV: {" << key << ": {" << value
                  << "} }." << std::endl;
#endif
        ps.put_or_replace(key, fhicl_consume(value), InProlog);
        break;
      }
      case '[': {
        // list member
        next_char = find_matching_bracket(fhicl, '[', next_not_space);
        std::string value =
            fhicl.substr(next_not_space, (next_char + 1) - next_not_space);
        ps.put_or_replace(key, value);
#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
        std::cout << "[INFO]: Found sequence KV: {" << key << ": [" << value
                  << "]}." << std::endl;
#endif
        break;
      }
      case '\"': {
        // string member
        next_char = fhicl.find_first_of("\"", next_not_space + 1);
        std::string value =
            fhicl.substr(next_not_space + 1, next_char - (next_not_space + 1));
        ps.put_or_replace(key, value);
#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
        std::cout << "[INFO]: Found KV: {" << key << ":" << value << "}."
                  << std::endl;
#endif
        break;
      }
      case '@': {
        std::string nil = fhicl.substr(next_not_space + 1, 3);
        if (nil == "nil") {
          ps.put_or_replace(key, std::string("@nil"));
          next_char = next_not_space + 4;
          break;
        }

        // Otherwise assume it is a fhicl directive
        next_char = fhicl.find_first_of(":", next_not_space + 1);
        std::string directive =
            fhicl.substr(next_not_space + 1, next_char - (next_not_space + 1));

        if (next_char)

          if (directive == "local") {
            // Get document parameter set
          } else if (directive == "table") {
            throw;
          } else if (directive == "sequence") {
            throw;
          } else {
            std::cout
                << "[ERROR]: Unknown fhicl directive: "
                << std::quoted(directive)
                << ", expecting one of \"local\", \"table\", or \"sequence\"."
                << std::endl;
            throw;
          }

        std::string local = fhicl.substr(next_not_space + 1, 7);
      }
      default: {
        next_char = fhicl.find_first_of(" ", next_not_space + 1);
        std::string value =
            fhicl.substr(next_not_space, next_char - next_not_space);
        ps.put_or_replace(key, value);
#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
        std::cout << "[INFO]: Found KV: {" << key << ":" << value << "}."
                  << std::endl;
#endif
        break;
      }
      }
    }
#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
    std::cout << "After reading value, next_char = " << next_char << " = \'"
              << fhicl[next_char] << "\'." << std::endl;
#endif
    if (next_char != std::string::npos) {
      read_ptr = fhicl.find_first_not_of(" ", next_char + 1);
    } else {
      read_ptr = std::string::npos;
    }
  }

  return ps;
}

ParameterSet parse_fhicl_from_string(std::string const &str) {

  fhicl_file_preproc_t fhicl_lines = ParseToVect<std::string>(str, "\n");
  fhicl_file_t fhicl = fhiclpp(fhicl_lines);
  return fhicl_consume(fhicl);
}
ParameterSet parse_fhicl_from_file(std::string const &str) {

  fhicl_file_preproc_t fhicl_lines = load_fhicl_file(str);
  fhicl_file_t fhicl = fhiclpp(fhicl_lines);
  return fhicl_consume(fhicl);
}
} // namespace fhicl

#endif
