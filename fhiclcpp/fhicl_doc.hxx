#ifndef FHICLCPP_SIMPLE_FHICLCPP_FHICL_DOC_H_SEEN
#define FHICLCPP_SIMPLE_FHICLCPP_FHICL_DOC_H_SEEN

#include "fhiclcpp/exception.hxx"

#include "string_parsers/utility.hxx"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

// Unix
#include <dirent.h>
#include <unistd.h>

namespace fhicl {

struct fhicl_doc_line_point {
  size_t line_no;
  size_t character;
  bool isend() const { return line_no == std::numeric_limits<size_t>::max(); }
  bool isendofline() const { return character == std::string::npos; }
  bool isofftheend() const { return isend() || isendofline(); }
  void setend() {
    line_no = std::numeric_limits<size_t>::max();
    character = std::string::npos;
  }
};

std::ostream &operator<<(std::ostream &os, fhicl_doc_line_point const &lp) {
  return os << "{ l: " << lp.line_no << ", c:" << lp.character << " }";
}

bool operator<(fhicl_doc_line_point const &l, fhicl_doc_line_point const &r) {
  if ((l.isend() && r.isendofline() && !r.isend()) ||
      (r.isend() && l.isendofline() && !l.isend())) {
    return true;
  }
  if (l.line_no < r.line_no) {
    return true;
  } else if (l.line_no == r.line_no) {
    if (l.isend()) {
      return false;
    }
    if (l.character < r.character) {
      return true;
    }
    return false;
  }
  return false;
}
bool operator==(fhicl_doc_line_point const &l, fhicl_doc_line_point const &r) {
  return !(l < r) && !(r < l);
}

struct fhicl_doc_line {
  size_t fileID;
  size_t file_line_no;
  std::string fhicl;
};

class fhicl_doc;

fhicl_doc read_doc(std::string const &filename);

class fhicl_doc : private std::vector<fhicl_doc_line> {
  std::vector<std::string> filenames;

  size_t get_filename_id(std::string const &filename = "") {
    if (!filename.size()) {
      return std::numeric_limits<size_t>::max();
    }
    for (size_t it = 0; it < filenames.size(); ++it) {
      if (filename == filenames[it]) {
        return it;
      }
    }
    filenames.push_back(filename);
    return (filenames.size() - 1);
  }

  // #define DEBUG_RESOLVE_INCLUDES

  void resolve_includes(std::vector<std::string> include_chain) {
    for (size_t i = 0; i < size(); ++i) {
#ifdef DEBUG_RESOLVE_INCLUDES
      std::cout << "[dri]: Checking line: " << std::quoted(at(i).fhicl)
                << " for include statements..." << std::endl;
#endif
      if (at(i).fhicl.find("#include \"") == 0) {
#ifdef DEBUG_RESOLVE_INCLUDES
        std::cout << "[dri]: Found one." << std::endl;
#endif
        size_t matchting_quote = at(i).fhicl.find_first_of('\"', 10);
        std::string inc_file_name =
            at(i).fhicl.substr(10, matchting_quote - 10);
        if (std::find(include_chain.begin(), include_chain.end(),
                      inc_file_name) != include_chain.end()) {

          throw include_loop()
              << "[ERROR]: Detected an include loop when trying to include: "
              << std::quoted(inc_file_name) << ". Current include chain: "
              << string_parsers::T2Str<std::vector<std::string>>(include_chain);
        }
        fhicl_doc inc = read_doc(inc_file_name);
        include_chain.push_back(inc_file_name);
        inc.resolve_includes(include_chain);

#ifdef DEBUG_RESOLVE_INCLUDES
        std::cout << "[dri]: Loaded one with: " << inc.size() << " lines."
                  << std::endl;
#endif
#ifdef DEBUG_RESOLVE_INCLUDES
        std::cout << "[dri]: Before insert: " << size() << std::endl;
#endif
        // Building fileID remap
        std::map<size_t, size_t> filenames_remap;
        for (size_t inc_fn_it = 0; inc_fn_it < inc.filenames.size();
             ++inc_fn_it) {
          filenames_remap[inc_fn_it] =
              get_filename_id(inc.filenames[inc_fn_it]);
        }

        // Apply remap so that inc line fileIDs are now correct
        for (size_t inc_ln_it = 0; inc_ln_it < inc.size(); ++inc_ln_it) {
          inc[inc_ln_it].fileID = filenames_remap[inc[inc_ln_it].fileID];
        }

        // remove the #include line
        erase(begin() + i);
        // add the rest of the other file
        insert(begin() + i, inc.begin(), inc.end());
#ifdef DEBUG_RESOLVE_INCLUDES
        std::cout << "[dri]: After insert: " << size() << std::endl;
#endif
      }
    }
  }

public:
  void push_back(std::string const &line, std::string const &filename = "",
                 size_t file_line_number = std::numeric_limits<size_t>::max()) {

    std::vector<fhicl_doc_line>::push_back(
        {get_filename_id(filename), file_line_number, line});
  }

  fhicl_doc_line_point validate_line_point(fhicl_doc_line_point lp) const {
    if (lp.line_no >= size()) {
      lp.setend();
    } else if (lp.character >= at(lp.line_no).fhicl.size()) {
      lp.character = std::string::npos;
    }
    return lp;
  }
  char get_char(fhicl_doc_line_point lp) const {
    lp = validate_line_point(lp);
    if (lp.isofftheend()) {
      return std::char_traits<char>::eof();
    }
    return at(lp.line_no).fhicl.at(lp.character);
  }
  std::string get_line(fhicl_doc_line_point lp) const {
    lp = validate_line_point(lp);
    if (lp.isend()) {
      return "EOF";
    }
    return at(lp.line_no).fhicl;
  }
  std::string get_line_info(fhicl_doc_line_point lp) const {
    lp = validate_line_point(lp);
    if (lp.isend()) {
      return "EOF";
    }
    fhicl_doc_line const &ln = at(lp.line_no);
    if (filenames.size() <= ln.fileID) {
      return "Unknown Origin";
    } else {
      std::stringstream ss("");
      ss << filenames[ln.fileID] << ":" << ln.file_line_no;
      return ss.str();
    }
  }
  std::string get_doc_info() const {
    std::stringstream ss("");
    ss << "{ " << size() << " lines, read from: "
       << string_parsers::T2Str<std::vector<std::string>>(filenames) << " }";
    return ss.str();
  }

  fhicl_doc_line_point advance_line(fhicl_doc_line_point lp,
                                    size_t n = 1) const {
    validate_line_point(lp);
    if (lp.isend()) {
      return lp;
    }
    lp.character = 0;
    lp.line_no += n;
    return validate_line_point(lp);
  }

  // #define DEBUG_ADVANCE

  fhicl_doc_line_point advance(fhicl_doc_line_point lp, size_t n = 1) const {
#ifdef DEBUG_ADVANCE
    std::cout << "Advancing " << lp << " by " << n << std::endl;
#endif
    lp = validate_line_point(lp);
#ifdef DEBUG_ADVANCE
    std::cout << "After validation " << lp << std::endl;
#endif
    while (lp.isendofline() && !lp.isend()) { // skip empty lines
      lp = advance_line(lp);
    }
#ifdef DEBUG_ADVANCE
    std::cout << "After endofline spin " << lp << std::endl;
#endif
    size_t n_to_advance = n;
    while (n_to_advance) {
#ifdef DEBUG_ADVANCE
      std::cout << "Still need to move " << n_to_advance << ", " << lp
                << std::endl;
#endif
      lp = validate_line_point(lp);
#ifdef DEBUG_ADVANCE
      std::cout << "After validation " << lp << std::endl;
#endif
      if (lp.isend()) {
#ifdef DEBUG_ADVANCE
        std::cout << "isend! " << std::endl;
#endif
        return lp;
      }
      while (lp.isendofline() && !lp.isend()) { // skip empty lines
        lp = advance_line(lp);
      }
#ifdef DEBUG_ADVANCE
      std::cout << "After endofline spin " << lp << std::endl;
#endif
      size_t n_left_in_line = at(lp.line_no).fhicl.size() - lp.character;
#ifdef DEBUG_ADVANCE
      std::cout << "N left in this line " << n_left_in_line << std::endl;
#endif
      if (n_to_advance <= n_left_in_line) {
        lp.character += n_to_advance;
#ifdef DEBUG_ADVANCE
        std::cout << "Advancing  " << n_to_advance << " to " << lp << std::endl;
#endif
        break;
      } else {
        n_to_advance -= n_left_in_line;
#ifdef DEBUG_ADVANCE
        std::cout << "Consuming entire line " << n_left_in_line << " to " << lp
                  << std::endl;
#endif
        lp = advance_line(lp);
#ifdef DEBUG_ADVANCE
        std::cout << "After advancing to the next line: " << lp << std::endl;
#endif
      }
    }
#ifdef DEBUG_ADVANCE
    std::cout << "After consuming all that I need to: " << lp << std::endl;
#endif
    lp = validate_line_point(lp);
#ifdef DEBUG_ADVANCE
    std::cout << "After validation: " << lp << std::endl;
#endif
    while (lp.isendofline() && !lp.isend()) { // skip empty lines
      lp = advance_line(lp);
    }
#ifdef DEBUG_ADVANCE
    std::cout << "After endofline spin: " << lp << std::endl;
#endif
    return lp;
  }

  fhicl_doc subdoc(fhicl_doc_line_point begin,
                   fhicl_doc_line_point end = {
                       std::numeric_limits<size_t>::max(),
                       std::string::npos}) const {
    fhicl_doc_line_point file_ptr = validate_line_point(begin);
    end = validate_line_point(end);

    if (file_ptr.isend()) {
      return fhicl_doc();
    }
    if (end < file_ptr) {
      return fhicl_doc();
    }

    fhicl_doc fd;
    // copy over history metadata
    fd.filenames = filenames;
    if (file_ptr.character) {
      fhicl_doc_line const &cline = at(file_ptr.line_no);
      fd.std::vector<fhicl_doc_line>::push_back(
          fhicl_doc_line{cline.fileID, cline.file_line_no,
                         cline.fhicl.substr(file_ptr.character)});
      file_ptr = advance_line(file_ptr);
    }
    while (!file_ptr.isend() && file_ptr.line_no < end.line_no) {
      fhicl_doc_line const &cline = at(file_ptr.line_no);
      fd.std::vector<fhicl_doc_line>::push_back(
          fhicl_doc_line{cline.fileID, cline.file_line_no, cline.fhicl});
      file_ptr = advance_line(file_ptr);
    }
    if (!file_ptr.isofftheend() && (file_ptr.character < end.character)) {
      fhicl_doc_line const &cline = at(file_ptr.line_no);
      fd.std::vector<fhicl_doc_line>::push_back(
          fhicl_doc_line{cline.fileID, cline.file_line_no,
                         cline.fhicl.substr(0, end.character)});
    }
    return fd;
  }
  std::string substr(fhicl_doc_line_point begin,
                     fhicl_doc_line_point end = {
                         std::numeric_limits<size_t>::max(),
                         std::string::npos}) const {

    fhicl_doc_line_point file_ptr = validate_line_point(begin);
    end = validate_line_point(end);

    if (file_ptr.isend()) {
      return "";
    }
    if (file_ptr.isendofline()) {
      file_ptr = advance_line(file_ptr);
    }
    if (end < file_ptr) {
      return "";
    }

    std::stringstream ss("");
    if (file_ptr.line_no == end.line_no) {
      return at(file_ptr.line_no)
          .fhicl.substr(file_ptr.character,
                        (end.character - file_ptr.character));
    } else if (file_ptr.character) {
      std::string line = at(file_ptr.line_no).fhicl.substr(file_ptr.character);
      if (line.size()) {
        ss << line << " ";
      }
      file_ptr = advance_line(file_ptr);
    }

    size_t nlinesread = 0;
    while (file_ptr.line_no < end.line_no) {
      std::string line = at(file_ptr.line_no).fhicl;
      if (line.size()) {
        ss << line << " ";
      }
      file_ptr = advance_line(file_ptr);
      nlinesread++;
    }

    if (nlinesread && !file_ptr.isofftheend() &&
        (file_ptr.character < end.character)) {
      std::string line = at(file_ptr.line_no).fhicl.substr(0, end.character);
      if (line.size()) {
        ss << line << " ";
      }
      file_ptr = advance_line(file_ptr);
    }

    std::string str = ss.str();
    if (!str.size()) {
      return "";
    }
    return str.substr(0, str.size() - 1);
  }

  // #define DEBUG_FIND

  fhicl_doc_line_point find_first_of(std::string const &search_chars,
                                     fhicl_doc_line_point const &begin = {0, 0},
                                     bool allow_newline = true) const {
#ifdef DEBUG_FIND
    std::cout << "[fnd] Searching for \"" << search_chars << "\" from " << begin
              << " in \"" << get_line(begin) << "\"" << std::endl;
#endif
    fhicl_doc_line_point search = validate_line_point(begin);
#ifdef DEBUG_FIND
    std::cout << "[fnd] After validation \"" << search_chars << "\" from "
              << begin << " in \"" << get_line(begin) << "\"" << std::endl;
#endif
    while (!search.isend()) {
      size_t pos = at(search.line_no)
                       .fhicl.find_first_of(search_chars, search.character);
#ifdef DEBUG_FIND
      std::cout << "[fnd] Next matching character at \"" << pos << "\" from "
                << search << " in \"" << get_line(search) << "\"" << std::endl;
#endif
      if (pos == std::string::npos) {
        if (!allow_newline) { // return the end of this line
          search.character = std::string::npos;
          return validate_line_point(search);
        }
#ifdef DEBUG_FIND
        std::cout << "[fnd] Checking next line..." << std::endl;
#endif
        search = advance_line(search);
      } else { // we have the character on this line
        search.character = pos;
#ifdef DEBUG_FIND
        std::cout << "[fnd] Got it!" << std::endl;
#endif
        break;
      }
    }

    return validate_line_point(search);
  }
  fhicl_doc_line_point find_first_of(char search_char,
                                     fhicl_doc_line_point const &begin = {0, 0},
                                     bool allow_newline = true) const {
    std::string char_str = "";
    char_str += search_char;
    return find_first_of(char_str, begin, allow_newline);
  }

  fhicl_doc_line_point
  find_first_not_of(std::string const &search_chars,
                    fhicl_doc_line_point const &begin = {0, 0},
                    bool allow_newline = true) const {
#ifdef DEBUG_FIND
    std::cout << "[fnd] Searching for not \"" << search_chars << "\" from "
              << begin << " in \"" << get_line(begin) << "\"" << std::endl;
#endif
    fhicl_doc_line_point search = validate_line_point(begin);
#ifdef DEBUG_FIND
    std::cout << "[fnd] After validation, looking for not \"" << search_chars
              << "\" from " << search << " in \"" << get_line(search) << "\""
              << std::endl;
#endif
    while (!search.isend()) {
      size_t pos = at(search.line_no)
                       .fhicl.find_first_not_of(search_chars, search.character);
#ifdef DEBUG_FIND
      std::cout << "[fnd] Next matching character at \"" << pos << "\" from "
                << search << " in \"" << get_line(search) << "\"" << std::endl;
#endif
      if (pos == std::string::npos) {
        if (!allow_newline) { // return the end of this line
          search.character = std::string::npos;
          return validate_line_point(search);
        }
#ifdef DEBUG_FIND
        std::cout << "[fnd] Checking next line..." << std::endl;
#endif
        search.line_no++;
        search.character = 0;
        search = validate_line_point(search);
      } else { // we have the character on this line
        search.character = pos;
#ifdef DEBUG_FIND
        std::cout << "[fnd] Got it!" << std::endl;
#endif
        break;
      }
    }
    return validate_line_point(search);
  }

  fhicl_doc_line_point
  find_first_not_of(char search_char,
                    fhicl_doc_line_point const &begin = {0, 0},
                    bool allow_newline = true) const {
    std::string char_str = "";
    char_str += search_char;
    return find_first_not_of(char_str, begin, allow_newline);
  }
  // public method that invokes the recursive one
  void resolve_includes() { resolve_includes({}); }

  std::string to_string(bool with_newlines = false,
                        bool with_line_info = false) const {
    std::stringstream ss("");

    for (size_t i = 0; i < size(); ++i) {
      fhicl_doc_line_point l;
      l.line_no = i;
      if (with_line_info) {
        ss << get_line_info(l) << "  |  ";
      }
      ss << get_line(l) << " ";

      if (with_newlines) {
        ss << std::endl;
      }
    }

    return ss.str();
  }
};

// #define DEBUG_FIND_MATCHING_BRACKET

#ifdef DEBUG_FIND_MATCHING_BRACKET
std::string indent = "";
#endif

fhicl_doc_line_point
find_matching_bracket(fhicl_doc &doc, char open_bracket = '{',
                      char close_bracket = '}',
                      fhicl_doc_line_point begin = {0, 0}) {
  begin = doc.validate_line_point(begin);

  if (begin.isofftheend()) {
    throw internal_error() << "[ERROR]: Starting position " << begin
                           << ", doesn't exist in doc " << doc.get_doc_info();
  }

  if (doc.get_char(begin) != open_bracket) {
    throw internal_error()
        << "[ERROR]: Bad search starting position, expected to find \""
        << open_bracket << "\", but found \"" << doc.get_line(begin) << "\"";
  }

#ifdef DEBUG_FIND_MATCHING_BRACKET
  std::cout << indent << "Searching for " << close_bracket << " matching "
            << open_bracket << " from " << begin << std::endl;
#endif

  fhicl_doc_line_point search_from = doc.advance(begin);

#ifdef DEBUG_FIND_MATCHING_BRACKET
  std::cout << indent << "Next good char at " << search_from << " = \""
            << doc.get_line(search_from) << "\"." << std::endl;
#endif

  fhicl_doc_line_point next_match =
      doc.find_first_of(close_bracket, search_from);

#ifdef DEBUG_FIND_MATCHING_BRACKET
  std::cout << indent << "Next matching close at " << next_match << " = \""
            << doc.get_line(next_match) << "\"." << std::endl;
#endif

  fhicl_doc_line_point next_bracket_open;
  std::pair<char, char> brackets{'\0', '\0'};

  next_bracket_open.setend();
  for (auto &bracket_types : std::vector<std::pair<char, char>>{
           {open_bracket, close_bracket}, {'\"', '\"'}}) {
    fhicl_doc_line_point bro =
        doc.find_first_of(bracket_types.first, search_from);
    if (bro < next_bracket_open) {
      brackets = bracket_types;
      next_bracket_open = bro;
    }
  }

#ifdef DEBUG_FIND_MATCHING_BRACKET
  if (!next_bracket_open.isofftheend()) {
    std::cout << indent << "Found opening bracket of type " << brackets.first
              << " at " << next_bracket_open << " = \""
              << doc.get_line(next_bracket_open) << "\"." << std::endl;
  }
#endif

  while (next_bracket_open < next_match) {

#ifdef DEBUG_FIND_MATCHING_BRACKET
    std::cout << indent
              << "Searching for matching close bracket: " << brackets.second
              << " from " << next_bracket_open << " = \""
              << doc.get_line(next_bracket_open) << "\"." << std::endl;
    indent = indent + "  ";
#endif

    fhicl_doc_line_point next_bracket_open_match = find_matching_bracket(
        doc, brackets.first, brackets.second, next_bracket_open);

#ifdef DEBUG_FIND_MATCHING_BRACKET
    indent = indent.substr(2);
    std::cout << indent << "Found closing bracket of type " << brackets.second
              << " at " << next_bracket_open_match << " = \""
              << doc.get_line(next_bracket_open_match) << "\"." << std::endl;
#endif

    search_from = doc.advance(next_bracket_open_match);

#ifdef DEBUG_FIND_MATCHING_BRACKET
    std::cout << indent << "Next good char \"" << doc.get_line(search_from)
              << "\" at " << search_from << std::endl;
#endif

    next_match = doc.find_first_of(close_bracket, search_from);

#ifdef DEBUG_FIND_MATCHING_BRACKET
    std::cout << indent << "Found main closing bracket of type "
              << close_bracket << " at " << next_match << " = \""
              << doc.get_line(next_match) << "\"." << std::endl;
#endif

    next_bracket_open.setend();
    for (auto &bracket_types : std::vector<std::pair<char, char>>{
             {open_bracket, close_bracket}, {'\"', '\"'}}) {
      fhicl_doc_line_point bro =
          doc.find_first_of(bracket_types.first, search_from);
      if (bro < next_bracket_open) {
        brackets = bracket_types;
        next_bracket_open = bro;
      }
    }

#ifdef DEBUG_FIND_MATCHING_BRACKET
    if (!next_bracket_open.isofftheend()) {
      std::cout << indent << "Found opening bracket of type " << brackets.first
                << " at " << next_bracket_open << " = \""
                << doc.get_line(next_bracket_open)
                << "\" (Next matching closing bracket at: " << next_match
                << ")." << std::endl;
    }
#endif
  }

  if (next_match.isend()) {
    throw malformed_document()
        << "[ERROR]: Failed to find matching bracket to " << begin << ", from "
        << doc.get_line_info(begin);
  }

#ifdef DEBUG_FIND_MATCHING_BRACKET
  std::cout << indent << "Returning matching close " << doc.get_char(next_match)
            << " at " << next_match << " = \"" << doc.get_line(next_match)
            << "\"." << std::endl;
#endif
  return next_match;
}

// #define DEBUG_OPEN_FHICL_FILE

std::unique_ptr<std::ifstream> open_fhicl_file(std::string const &filename) {

#ifdef DEBUG_OPEN_FHICL_FILE
  std::cout << "[open_fhicl_file]: Trying to resolve " << std::quoted(filename)
            << std::endl;
#endif

  if (filename.find_last_of('/') !=
      std::string::npos) { // If there are slashes, assume that it is a relative
                           // path.
#ifdef DEBUG_OPEN_FHICL_FILE
    std::cout
        << "[open_fhicl_file]: Found a forward slash, assuming a relative path."
        << std::endl;
#endif
    return std::make_unique<std::ifstream>(filename);
  }

  char const *fhicl_file_path = getenv("FHICL_FILE_PATH");
  if (!fhicl_file_path) { // Try and open the file anyway assuming that it is
// relative
#ifdef DEBUG_OPEN_FHICL_FILE
    std::cout << "[open_fhicl_file]: FHICL_FILE_PATH is not defined, trying to "
                 "treat it as a local filename."
              << std::endl;
#endif
    return std::make_unique<std::ifstream>(filename);
  }

  for (std::string const &path : string_parsers::ParseToVect<std::string>(
           fhicl_file_path, ":", false, true)) {
#ifdef DEBUG_OPEN_FHICL_FILE
    std::cout << "[open_fhicl_file]: Searching directory from FHICL_FILE_PATH: "
              << std::quoted(path) << std::endl;
#endif
    DIR *dir = opendir(path.c_str());
    struct dirent *ent;
    if (dir != NULL) {
      while ((ent = readdir(dir)) != NULL) {
#ifdef DEBUG_OPEN_FHICL_FILE
        std::cout << "[open_fhicl_file]: Checking entry: "
                  << std::quoted(ent->d_name) << std::endl;
#endif
        if (std::string(ent->d_name) == filename) {
#ifdef DEBUG_OPEN_FHICL_FILE
          std::cout << "[open_fhicl_file]: Attempting to open : "
                    << std::quoted(string_parsers::ensure_trailing_slash(path) +
                                   filename)
                    << std::endl;
#endif
          return std::make_unique<std::ifstream>(
              string_parsers::ensure_trailing_slash(path) + filename);
        }
      }
      closedir(dir);
    } else { // Couldn't open directory
      std::cout << "[WARN]: Failed to search directory: " << std::quoted(path)
                << " found in FHICL_FILE_PATH. opendir failed with error: "
                << std::quoted(std::strerror(errno)) << std::endl;
    }
  }
  return nullptr;
}

fhicl_doc read_doc(std::string const &filename) {
  std::unique_ptr<std::ifstream> ifs = open_fhicl_file(filename);
  if (!ifs || !ifs->good()) {
    throw file_does_not_exist() << "[ERROR]: File: " << std::quoted(filename)
                                << " could not be opened for reading.";
  }
  std::string line;
  fhicl_doc doc;
  size_t ctr = 0;
  while (std::getline(*ifs, line)) {
    string_parsers::trim(line);
    doc.push_back(line, filename, ctr);
    ctr++;
  }
  return doc;
}

} // namespace fhicl
#endif
