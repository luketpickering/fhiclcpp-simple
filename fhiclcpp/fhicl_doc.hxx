#ifndef FHICLCPP_SIMPLE_FHICLCPP_FHICL_DOC_H_SEEN
#define FHICLCPP_SIMPLE_FHICLCPP_FHICL_DOC_H_SEEN

#include "fhiclcpp/exception.hxx"

#include "string_parsers/traits.hxx"
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
  bool isbeginofline() const { return character == 0; }
  bool isend() const { return line_no == std::numeric_limits<size_t>::max(); }
  bool isendofline() const { return character == std::string::npos; }
  bool isofftheend() const { return isend() || isendofline(); }

  static fhicl_doc_line_point end() {
    return fhicl_doc_line_point{std::numeric_limits<size_t>::max(),
                                std::string::npos};
  }
  static fhicl_doc_line_point begin() { return fhicl_doc_line_point{0, 0}; }
};

inline std::ostream &operator<<(std::ostream &os,
                                fhicl_doc_line_point const &lp) {
  return os << "{ l: " << lp.line_no << ", c:" << lp.character << " }";
}

inline bool operator<(fhicl_doc_line_point const &l,
                      fhicl_doc_line_point const &r) {
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
inline bool operator==(fhicl_doc_line_point const &l,
                       fhicl_doc_line_point const &r) {
  return !(l < r) && !(r < l);
}
inline bool operator!=(fhicl_doc_line_point const &l,
                       fhicl_doc_line_point const &r) {
  return !(l == r);
}
inline bool operator>(fhicl_doc_line_point const &l,
                      fhicl_doc_line_point const &r) {
  return !((l < r) || (l == r));
}
inline bool operator>=(fhicl_doc_line_point const &l,
                       fhicl_doc_line_point const &r) {
  return !(l < r);
}

struct fhicl_doc_range {
  fhicl_doc_line_point begin;
  fhicl_doc_line_point end;
  static fhicl_doc_range all() {
    return {fhicl_doc_line_point::begin(), fhicl_doc_line_point::end()};
  }
};

struct fhicl_doc_line {
  size_t fileID;
  size_t file_line_no;
  std::string fhicl;
};

class fhicl_doc;

inline fhicl_doc read_doc(std::string const &filename);

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
        fhicl_doc inc;
        try {
          inc = read_doc(inc_file_name);
        } catch (file_does_not_exist &e) {
          throw file_does_not_exist()
              << "[ERROR]: When attempting to resolving include statement, "
                 "found at "
              << get_line_info(fhicl_doc_line_point{i, 0}) << " = "
              << std::quoted(get_line(fhicl_doc_line_point{i, 0}))
              << " caught exception:\n  --" << e.what() << "\n\n File "
              << std::quoted(inc_file_name)
              << " not found, or FHICL_FILE_PATH (="
              << (getenv("FHICL_FILE_PATH")
                      ? std::quoted(getenv("FHICL_FILE_PATH"))
                      : std::quoted(""))
              << ") improperly defined.";
        }
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

  // #define DEBUG_VALIDATE_LINE_POINT
  fhicl_doc_line_point validate_line_point(fhicl_doc_line_point lp) const {
#ifdef DEBUG_VALIDATE_LINE_POINT
    std::cout << "      [Validlp]: Validating: " << lp << std::endl;
    if (size() > lp.line_no) {
      std::cout << at(lp.line_no).fhicl << std::endl;
    }
#endif
    if (lp.line_no >= size()) {
#ifdef DEBUG_VALIDATE_LINE_POINT
      std::cout << "      [Validlp]: lp LineNo = " << lp.line_no
                << ",  but doc.size() = " << size() << std::endl;
#endif
      lp = fhicl_doc_line_point::end();
    } else if (lp.character >= at(lp.line_no).fhicl.size()) {
#ifdef DEBUG_VALIDATE_LINE_POINT
      std::cout << "      [Validlp]: lp character = " << lp.character
                << ", on line " << lp.line_no << " at(" << lp.line_no
                << ").fhicl.size() ==  " << at(lp.line_no).fhicl.size()
                << std::endl;
#endif
      // if ((lp.line_no == 0) &&
      //     !at(lp.line_no).fhicl.size()) { // Beginning of an empty first
      //                                     // line shouldn't be
      //   // set as the end of an empty first line
      //   return validate_line_point(fhicl_doc_line_point::begin());
      // } else {
      lp.character = std::string::npos;
      // }
    }
    return lp;
  }
  bool line_is_empty(fhicl_doc_line_point lp) const {
    lp = validate_line_point(lp);
    if (lp.isend()) {
      return true;
    }
    return !at(lp.line_no).fhicl.size();
  }
  fhicl_doc_line_point get_first_line_point() {
    fhicl_doc_line_point lp = fhicl_doc_line_point::begin();
    while ((lp.line_no < size()) && !at(lp.line_no).fhicl.size()) {
      lp.line_no++;
    }
    if (lp.line_no == size()) {
      return fhicl_doc_line_point::end();
    }
    return lp;
  }
  bool isbegin(fhicl_doc_line_point l) {
    if (!size()) {
      return true;
    }
  }
  bool isend(fhicl_doc_line_point l) {
    if (!size()) {
      return true;
    }
  }
  // #define DEBUG_ARE_EQUIVALENT
  bool are_equivalent(fhicl_doc_line_point l, fhicl_doc_line_point r) {
    l = validate_line_point(l);
    r = validate_line_point(r);
#ifdef DEBUG_ARE_EQUIVALENT
    std::cout << "[EQUIV]: " << l << " = " << std::quoted(get_line(l, true))
              << " ?= " << r << " = " << std::quoted(get_line(r, true))
              << std::endl;
#endif
    if (l == r) {
      return true;
    }
    if (r < l) {
      fhicl_doc_line_point swap = l;
      l = r;
      r = swap;
#ifdef DEBUG_ARE_EQUIVALENT
      std::cout << "  [EQUIV]: Swapped: " << l << " = "
                << std::quoted(get_line(l, true)) << " ?= " << r << " = "
                << std::quoted(get_line(r, true)) << std::endl;
#endif
    }
#ifdef DEBUG_ARE_EQUIVALENT
    std::cout << "  [EQUIV]: " << advance(l, 0) << " = "
              << std::quoted(get_line(advance(l, 0), true)) << " ?= " << r
              << " = " << std::quoted(get_line(r, true)) << std::endl;
#endif
    if (advance(l, 0) == advance(r, 0)) {
      return true;
    }
#ifdef DEBUG_ARE_EQUIVALENT
    std::cout << "  [EQUIV]: " << l << " = " << std::quoted(get_line(l, true))
              << " ?= " << rewind(r, 0) << " = "
              << std::quoted(get_line(rewind(r, 0), true)) << std::endl;
#endif
    if (rewind(l, 0) == rewind(r, 0)) {
      return true;
    }
    return false;
  }

  fhicl_doc_line_point get_last_line_point() const {
    if (!size()) {
      return validate_line_point(fhicl_doc_line_point::begin());
    }
    fhicl_doc_line_point lp{size() - 1, back().fhicl.size() - 1};
    while (line_is_empty(lp)) {
      lp = rewind_line(lp);
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

  std::string get_line(fhicl_doc_line_point lp, bool color = false) const {
    lp = validate_line_point(lp);
    if (lp.isend()) {
      return "EOF";
    }
    if (!color) {
      return at(lp.line_no).fhicl;
    }
    std::stringstream ss("");
    for (size_t i = 0; i < at(lp.line_no).fhicl.size(); ++i) {
      if (i == lp.character) {
        ss << "\033[31;47m";
      }
      ss << at(lp.line_no).fhicl[i];
      if (i == lp.character) {
        ss << "\033[0m";
      }
    }
    if (lp.isendofline()) {
      ss << "\033[31;47m\\EOL\033[0m";
    }
    return ss.str();
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

  fhicl_doc_line_point rewind_line(fhicl_doc_line_point lp,
                                   size_t n = 1) const {
    validate_line_point(lp);
    if (lp.line_no < n) {
      return validate_line_point(fhicl_doc_line_point::begin());
    }
    lp.line_no -= n;
    lp.character = (at(lp.line_no).fhicl.size() -
                    1); // if empty line will be set to the end of the line,
                        // if on first line that is empty will be set to
                        // isbegin by validate_line_point
    return validate_line_point(lp);
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

  // #define DEBUG_REWIND
  fhicl_doc_line_point rewind(fhicl_doc_line_point lp, size_t n = 1) const {
    if (lp.isend()) {
      lp = get_last_line_point();
    }
#ifdef DEBUG_REWIND
    std::cout << "Rewinding " << lp << " -- " << std::quoted(get_line(lp, true))
              << " by " << n << std::endl;
#endif
    lp = validate_line_point(lp);
#ifdef DEBUG_REWIND
    std::cout << "    After validation " << lp << " -- "
              << std::quoted(get_line(lp, true)) << std::endl;
#endif

    if ((lp != fhicl_doc_line_point::begin()) && !line_is_empty(lp) &&
        lp.isendofline()) { // if you get passed a eol line point on a
                            // non-empty line, move it back to the last
                            // character
      lp.character = (at(lp.line_no).fhicl.size() - 1);
    }
#ifdef DEBUG_REWIND
    std::cout << "    Check EOL " << lp << " -- "
              << std::quoted(get_line(lp, true)) << std::endl;
#endif

    while (line_is_empty(lp) && (lp.line_no != 0)) { // If at end of empty line
      lp = rewind_line(lp);
    }
#ifdef DEBUG_REWIND
    std::cout << "    After empty_line spin " << lp << " -- "
              << std::quoted(get_line(lp, true)) << std::endl;
#endif
    size_t n_to_rewind = n;
    while (n_to_rewind) {
#ifdef DEBUG_REWIND
      std::cout << "    Still need to move " << n_to_rewind << ", " << lp
                << std::endl;
#endif
      lp = validate_line_point(lp);
#ifdef DEBUG_REWIND
      std::cout << "    After validation " << lp << " -- "
                << std::quoted(get_line(lp, true)) << std::endl;
#endif
      if (lp == fhicl_doc_line_point::begin()) {
#ifdef DEBUG_REWIND
        std::cout << "    isbegin! " << std::endl;
#endif
        return validate_line_point(lp); // incase the first line is empty
      }
      while (line_is_empty(lp) &&
             (lp.line_no != 0)) { // If at end of empty line
        lp = rewind_line(lp);
      }
#ifdef DEBUG_REWIND
      std::cout << "    After empty line spin " << lp << " -- "
                << std::quoted(get_line(lp, true)) << std::endl;
#endif
      size_t n_left_in_line = lp.character;
#ifdef DEBUG_REWIND
      std::cout << "    N left in this line " << n_left_in_line << std::endl;
#endif
      if (n_to_rewind <= n_left_in_line) {
        lp.character -= n_to_rewind;
#ifdef DEBUG_REWIND
        std::cout << "    Rewinding  " << n_to_rewind << " to " << lp
                  << std::endl;
#endif
        break;
      } else {
        n_to_rewind -= (n_left_in_line + 1);
#ifdef DEBUG_REWIND
        std::cout << "    Consuming entire line " << n_left_in_line << " to "
                  << lp << " -- " << std::quoted(get_line(lp, true))
                  << std::endl;
#endif
        lp = rewind_line(lp);
#ifdef DEBUG_REWIND
        std::cout << "    After rewinding to the previous line: " << lp
                  << std::endl;
#endif
      }
    }
#ifdef DEBUG_REWIND
    std::cout << "    After consuming all that I need to: " << lp << " -- "
              << std::quoted(get_line(lp, true)) << std::endl;
#endif
    lp = validate_line_point(lp);
#ifdef DEBUG_REWIND
    std::cout << "    After validation: " << lp << " -- "
              << std::quoted(get_line(lp, true)) << std::endl;
#endif
    while (line_is_empty(lp) && (lp.line_no != 0)) { // If at end of empty line
      lp = rewind_line(lp);
    }
#ifdef DEBUG_REWIND
    std::cout << "    After empty line spin: " << lp << " -- "
              << std::quoted(get_line(lp, true)) << std::endl;
#endif
    return lp;
  }

  // #define DEBUG_ADVANCE

  fhicl_doc_line_point advance(fhicl_doc_line_point lp, size_t n = 1) const {

#ifdef DEBUG_ADVANCE
    std::cout << "Advancing " << lp << " -- " << std::quoted(get_line(lp, true))
              << " by " << n << std::endl;
#endif
    lp = validate_line_point(lp);
#ifdef DEBUG_ADVANCE
    std::cout << "    After validation " << lp << " -- "
              << std::quoted(get_line(lp, true)) << std::endl;
#endif
    while (lp.isendofline() && !lp.isend()) { // skip empty lines
      lp = advance_line(lp);
    }
#ifdef DEBUG_ADVANCE
    std::cout << "    After empty line spin " << lp << " -- "
              << std::quoted(get_line(lp, true)) << std::endl;
#endif
    size_t n_to_advance = n;
    while (n_to_advance) {
#ifdef DEBUG_ADVANCE
      std::cout << "    Still need to move " << n_to_advance << ", " << lp
                << std::endl;
#endif
      lp = validate_line_point(lp);
#ifdef DEBUG_ADVANCE
      std::cout << "    After validation " << lp << " -- "
                << std::quoted(get_line(lp, true)) << std::endl;
#endif
      if (lp.isend()) {
#ifdef DEBUG_ADVANCE
        std::cout << "    isend! " << std::endl;
#endif
        return lp;
      }
      while (lp.isendofline() && !lp.isend()) { // skip empty lines
        lp = advance_line(lp);
      }
#ifdef DEBUG_ADVANCE
      std::cout << "    After empty line spin " << lp << " -- "
                << std::quoted(get_line(lp, true)) << std::endl;
#endif
      size_t n_left_in_line = at(lp.line_no).fhicl.size() - lp.character;
#ifdef DEBUG_ADVANCE
      std::cout << "    N left in this line " << n_left_in_line << std::endl;
#endif
      if (n_to_advance <= n_left_in_line) {
        lp.character += n_to_advance;
#ifdef DEBUG_ADVANCE
        std::cout << "    Advancing  " << n_to_advance << " to " << lp
                  << std::endl;
#endif
        break;
      } else {
        n_to_advance -= n_left_in_line;
#ifdef DEBUG_ADVANCE
        std::cout << "    Consuming entire line " << n_left_in_line << " to "
                  << lp << " -- " << std::quoted(get_line(lp, true))
                  << std::endl;
#endif
        lp = advance_line(lp);
#ifdef DEBUG_ADVANCE
        std::cout << "    After advancing to the next line: " << lp
                  << std::endl;
#endif
      }
    }
#ifdef DEBUG_ADVANCE
    std::cout << "    After consuming all that I need to: " << lp << " -- "
              << std::quoted(get_line(lp, true)) << std::endl;
#endif
    lp = validate_line_point(lp);
#ifdef DEBUG_ADVANCE
    std::cout << "    After validation: " << lp << " -- "
              << std::quoted(get_line(lp, true)) << std::endl;
#endif
    while (lp.isendofline() && !lp.isend()) { // skip empty lines
      lp = advance_line(lp);
    }
#ifdef DEBUG_ADVANCE
    std::cout << "    After empty line spin: " << lp << " -- "
              << std::quoted(get_line(lp, true)) << std::endl;
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

  fhicl_doc subdoc(fhicl_doc_range range) const {
    return subdoc(range.begin, range.end);
  }

  // #define DEBUG_SUBSTR

  std::string substr(fhicl_doc_line_point begin,
                     fhicl_doc_line_point end = {
                         std::numeric_limits<size_t>::max(),
                         std::string::npos}) const {

#ifdef DEBUG_SUBSTR
    std::cout << "--substr-- From " << begin << " -- "
              << std::quoted(get_line(begin, true)) << ", to " << end
              << std::quoted(get_line(end, true)) << std::endl;
#endif

    fhicl_doc_line_point file_ptr = validate_line_point(begin);
    end = validate_line_point(end);

#ifdef DEBUG_SUBSTR
    std::cout << "--substr-- After validation from " << file_ptr << " -- "
              << std::quoted(get_line(file_ptr, true)) << ", to " << end
              << std::quoted(get_line(end, true)) << std::endl;
#endif

    if (file_ptr.isend()) {
      return "";
    }
    if (file_ptr.isendofline()) {
#ifdef DEBUG_SUBSTR
      std::cout << "--substr-- Begin is at the end of a line, advancing: "
                << file_ptr << " -- " << std::quoted(get_line(file_ptr, true))
                << std::endl;
#endif
      file_ptr = advance_line(file_ptr);
    }
    if (end < file_ptr) {
      return "";
    }

    std::stringstream ss("");
    if (file_ptr.line_no == end.line_no) {
#ifdef DEBUG_SUBSTR
      std::cout << "--substr-- Begin(" << file_ptr << ") and end(" << end
                << ") are on the same line: returning "
                << std::quoted(
                       at(file_ptr.line_no)
                           .fhicl.substr(file_ptr.character,
                                         (end.character - file_ptr.character)))
                << std::endl;
#endif
      return at(file_ptr.line_no)
          .fhicl.substr(file_ptr.character,
                        (end.character - file_ptr.character));
    } else if (file_ptr.character) {
#ifdef DEBUG_SUBSTR
      std::cout << "--substr-- Have some, but not all characters on the first "
                   "line, taking "
                << std::quoted(
                       at(file_ptr.line_no).fhicl.substr(file_ptr.character))
                << std::endl;
#endif
      std::string line = at(file_ptr.line_no).fhicl.substr(file_ptr.character);
      if (line.size()) {
        ss << line << " ";
      }
      file_ptr = advance_line(file_ptr);
    }

    size_t nlinesread = 0;
    while (file_ptr.line_no < end.line_no) {
#ifdef DEBUG_SUBSTR
      std::cout << "--substr-- Taking all of line " << file_ptr
                << std::quoted(get_line(file_ptr, true)) << std::endl;
#endif
      std::string line = at(file_ptr.line_no).fhicl;
      if (line.size()) {
        ss << line << " ";
      }
      file_ptr = advance_line(file_ptr);
      nlinesread++;
    }

    if (!file_ptr.isofftheend() && (file_ptr.character < end.character)) {
#ifdef DEBUG_SUBSTR
      std::cout << "--substr-- Taking the end line up to " << end
                << std::quoted(get_line(end, true)) << std::endl;
#endif
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
#ifdef DEBUG_SUBSTR
    std::cout << "--substr-- Returning " << str.substr(0, str.size() - 1)
              << std::endl;
#endif
    return str.substr(0, str.size() - 1);
  }

  std::string substr(fhicl_doc_range range) const {
    return substr(range.begin, range.end);
  }

  // #define DEBUG_FIND

  fhicl_doc_line_point find_first_of(
      std::string const &search_chars,
      fhicl_doc_line_point const &begin = fhicl_doc_line_point::begin(),
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
  fhicl_doc_line_point find_first_of(
      char search_char,
      fhicl_doc_line_point const &begin = fhicl_doc_line_point::begin(),
      bool allow_newline = true) const {
    std::string char_str = "";
    char_str += search_char;
    return find_first_of(char_str, begin, allow_newline);
  }

  fhicl_doc_line_point find_first_not_of(
      std::string const &search_chars,
      fhicl_doc_line_point const &begin = fhicl_doc_line_point::begin(),
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

  fhicl_doc_line_point find_first_not_of(
      char search_char,
      fhicl_doc_line_point const &begin = fhicl_doc_line_point::begin(),
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

inline fhicl_doc_line_point find_matching_bracket(
    fhicl_doc const &doc, char open_bracket = '{', char close_bracket = '}',
    fhicl_doc_line_point begin = fhicl_doc_line_point::begin()) {
  begin = doc.validate_line_point(begin);

  if (begin.isofftheend()) {
    throw internal_error() << "[ERROR]: Starting position " << begin
                           << ", doesn't exist in doc " << doc.get_doc_info();
  }

  if (doc.get_char(begin) != open_bracket) {
    throw internal_error()
        << "[ERROR]: Bad search starting position, expected to find \""
        << open_bracket << "\", but found "
        << std::quoted(doc.get_line(begin, true)) << " from "
        << std::quoted(doc.get_line_info(begin));
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

  next_bracket_open = fhicl_doc_line_point::end();
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

    next_bracket_open = fhicl_doc_line_point::end();
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

// #define DEBUG_GET_LIST_ELEMENTS

inline std::vector<fhicl_doc_range> get_list_elements(fhicl_doc const &doc,
                                                      fhicl_doc_range range,
                                                      bool trim = false) {

  range.begin = doc.validate_line_point(range.begin);
#ifdef DEBUG_GET_LIST_ELEMENTS
  std::cout << "Begin at: " << range.begin << " -- "
            << std::quoted(doc.get_line(range.begin, true)) << std::endl;
#endif
  range.end = doc.validate_line_point(range.end);
#ifdef DEBUG_GET_LIST_ELEMENTS
  std::cout << "Go until: " << range.end << " -- "
            << std::quoted(doc.get_line(range.end, true)) << std::endl;
#endif

  fhicl_doc_line_point nextOccurence = range.begin;
  fhicl_doc_line_point searchZero = range.begin;
  std::vector<fhicl_doc_range> outV;

  static const std::map<char, char> type_care_brackets =
      string_rep_delim<std::vector<std::string>>::brackets();

  while (searchZero < range.end) {
#ifdef DEBUG_GET_LIST_ELEMENTS
    std::cout << "  Searching for next comma after: " << searchZero << " -- "
              << std::quoted(doc.get_line(searchZero, true)) << std::endl;
#endif
    nextOccurence = doc.find_first_of(',', searchZero);

#ifdef DEBUG_GET_LIST_ELEMENTS
    std::cout << "  Next comma at: " << nextOccurence << " -- "
              << std::quoted(doc.get_line(nextOccurence, true)) << std::endl;
#endif

    fhicl_doc_line_point bracketSearchZero = searchZero;

#ifdef DEBUG_GET_LIST_ELEMENTS
    std::cout << "  Searching for brackets from : " << searchZero << " -- "
              << std::quoted(doc.get_line(searchZero, true)) << std::endl;
#endif

    while ((!nextOccurence.isend()) && type_care_brackets.size()) {

      fhicl_doc_line_point nextBracket = fhicl_doc_line_point::end();
      std::pair<char, char> brackets{'\0', '\0'};
      for (auto const &bracket_pair : type_care_brackets) {
        fhicl_doc_line_point f =
            doc.find_first_of(bracket_pair.first, bracketSearchZero);
#ifdef DEBUG_GET_LIST_ELEMENTS
        std::cout << "  Next bracket of type " << bracket_pair.first
                  << " is at: " << f << " -- "
                  << std::quoted(doc.get_line(f, true)) << std::endl;
#endif
        if (f < nextBracket) {
#ifdef DEBUG_GET_LIST_ELEMENTS
          std::cout << "  + This is now the closest bracket at: " << f << " -- "
                    << std::quoted(doc.get_line(f, true)) << std::endl;
#endif
          nextBracket = f;
          brackets = bracket_pair;
        }
      }

#ifdef DEBUG_GET_LIST_ELEMENTS
      std::cout << "  Next bracket at: " << nextBracket
                << ", next comma at: " << nextOccurence << std::endl;
#endif
      if (nextBracket > nextOccurence) { // The next bracket that we care about
// is after the next comma
#ifdef DEBUG_GET_LIST_ELEMENTS
        std::cout << "  Breaking." << std::endl;
#endif
        break;
      } else if (nextBracket <
                 nextOccurence) { // The next bracket comes before the next
                                  // comma, the comma could be contained
                                  // within that bracket.
#ifdef DEBUG_GET_LIST_ELEMENTS
        std::cout << "  Next bracket is worrying, must check for matching "
                     "bracket of type: "
                  << brackets.second << std::endl;
#endif
        fhicl_doc_line_point matching_bracket = find_matching_bracket(
            doc, brackets.first, brackets.second, nextBracket);

        if (((brackets.first == '\"') || (brackets.first == '\'')) &&
            (matching_bracket >=
             fhicl_doc_line_point{nextBracket.line_no, std::string::npos})) {
          throw malformed_document()
              << "[ERROR]: When searching for matching quote to the one "
                 "found "
                 "at "
              << nextBracket << " on line "
              << std::quoted(doc.get_line(nextBracket, true))
              << " in document: " << std::quoted(doc.get_line_info(nextBracket))
              << " the end of the line was reached. Quoted strings cannot "
                 "span "
                 "multiple lines.";
        }

#ifdef DEBUG_GET_LIST_ELEMENTS
        std::cout << "  Found matching bracket at : " << matching_bracket
                  << " -- " << std::quoted(doc.get_line(matching_bracket, true))
                  << ", next comma is at: " << nextOccurence << std::endl;
#endif
        if (matching_bracket >
            nextOccurence) { // The matching bracket occurs after the next
                             // comma, therefore we should skip that comma and
                             // look for the next one.
#ifdef DEBUG_GET_LIST_ELEMENTS
          std::cout << "  Skipping this comma..." << std::endl;
#endif
          bracketSearchZero = doc.advance(matching_bracket);
#ifdef DEBUG_GET_LIST_ELEMENTS
          std::cout << " Continuing bracket search from: " << bracketSearchZero
                    << " -- "
                    << std::quoted(doc.get_line(bracketSearchZero, true))
                    << std::endl;
#endif
          nextOccurence = doc.find_first_of(',', bracketSearchZero);
#ifdef DEBUG_GET_LIST_ELEMENTS
          std::cout << " Updated next comma occurence to: " << nextOccurence
                    << " -- " << std::quoted(doc.get_line(nextOccurence, true))
                    << std::endl;
#endif
          if (doc.advance(matching_bracket) == nextOccurence) {
#ifdef DEBUG_GET_LIST_ELEMENTS
            std::cout << "  The bracket closes on the character before the "
                         "next comma, short circuiting."
                      << std::endl;
#endif
            break;
          }
          continue;

        } else if (matching_bracket < nextOccurence) { // That bracket ends
                                                       // before the next comma,
          // but there could be another one.
          if (doc.advance(matching_bracket) == nextOccurence) {
#ifdef DEBUG_GET_LIST_ELEMENTS
            std::cout << "  The bracket closes on the character before the "
                         "next comma, short circuiting."
                      << std::endl;
#endif
            break;
          }
          bracketSearchZero = doc.advance(matching_bracket);
#ifdef DEBUG_GET_LIST_ELEMENTS
          std::cout << "  The bracket closes before the next comma, searching "
                       "for any new worrying brackets from: "
                    << bracketSearchZero << std::endl;
#endif
          continue;
        } else if (matching_bracket.isend()) {
          throw malformed_document()
              << "[ERROR]: When searching for closing bracket that matches "
                 "the "
                 "opening one found at: "
              << nextBracket << " on line "
              << std::quoted(doc.get_line(nextBracket, true)) << " in document "
              << std::quoted(doc.get_line_info(nextBracket))
              << " hit the end of document.";

        } else {
          throw bizare_error()
              << "[ERROR]: Matching bracket occured at the same place as the "
                 "next comma, this shouldn't have happened.";
        }
      } else {
#ifdef DEBUG_GET_LIST_ELEMENTS
        std::cout << "  The next bracket opens after the next comma."
                  << std::endl;
#endif
        break;
      }
    }

#ifdef DEBUG_GET_LIST_ELEMENTS
    std::cout << "  The next comma found, which delimits a list item was "
                 "determined to be at: "
              << nextOccurence << std::endl;
#endif

    if (nextOccurence > range.end) {
      nextOccurence = range.end;
#ifdef DEBUG_GET_LIST_ELEMENTS
      std::cout << " The next comma ran passed the end of the allowed document "
                   "region. Ending list element search here."
                << std::endl;
#endif
    }
    outV.push_back({doc.validate_line_point(searchZero),
                    doc.validate_line_point(nextOccurence)});
#ifdef DEBUG_GET_LIST_ELEMENTS
    std::cout << "  Element determined as: " << searchZero << " -- "
              << nextOccurence << " = "
              << std::quoted(doc.substr(searchZero, nextOccurence))
              << std::endl;
#endif
    searchZero = doc.advance(nextOccurence);
  }
  if (trim) { // roll on begin and end pointers until they hit a non space
    // character
    for (fhicl_doc_range &r : outV) {
#ifdef DEBUG_GET_LIST_ELEMENTS
      std::cout << "Range begin = " << r.begin << " -- "
                << std::quoted(doc.get_line(r.begin, true)) << std::endl;
#endif
      // Move the pointers off any EOL
      if (r.begin.isendofline()) {
        r.begin = doc.advance(r.begin, 0);
      }
#ifdef DEBUG_GET_LIST_ELEMENTS
      std::cout << "  Range begin after EOL spin = " << r.begin << " -- "
                << std::quoted(doc.get_line(r.begin, true)) << std::endl;
#endif
#ifdef DEBUG_GET_LIST_ELEMENTS
      std::cout << "  Range end = " << r.end << " -- "
                << std::quoted(doc.get_line(r.end, true)) << std::endl;
#endif
      if (r.end.isendofline()) {
        r.end = doc.rewind(r.end, 0);
      }
#ifdef DEBUG_GET_LIST_ELEMENTS
      std::cout << "  Range end after EOL spin = " << r.end << " -- "
                << std::quoted(doc.get_line(r.end, true)) << std::endl;
#endif
      // Push the begin pointer forward until it hits a non-space character
      while (std::isspace(doc.get_char(r.begin)) && (r.begin != r.end)) {
#ifdef DEBUG_GET_LIST_ELEMENTS
        std::cout << "   >Range begin = " << r.begin << " -- "
                  << std::quoted(doc.get_line(r.begin, true)) << std::endl;
#endif
        r.begin = doc.advance(r.begin);
      }
#ifdef DEBUG_GET_LIST_ELEMENTS
      std::cout << "  Trimmed begin = " << r.begin << " -- "
                << std::quoted(doc.get_line(r.begin, true)) << std::endl;
#endif
      // Push the end pointer backward until it hits a non-space character
      fhicl_doc_line_point last_char = doc.rewind(r.end);
      while (std::isspace(doc.get_char(last_char)) && (r.begin != r.end)) {
#ifdef DEBUG_GET_LIST_ELEMENTS
        std::cout << "   >Range end = " << r.end << " -- "
                  << std::quoted(doc.get_line(r.end, true)) << std::endl;
#endif
        last_char = doc.rewind(last_char);
#ifdef DEBUG_GET_LIST_ELEMENTS
        std::cout << "   +Range end = " << r.end << " -- "
                  << std::quoted(doc.get_line(r.end, true)) << std::endl;
#endif
        r.end = doc.rewind(r.end);
#ifdef DEBUG_GET_LIST_ELEMENTS
        std::cout << "   -Range end = " << r.end << " -- "
                  << std::quoted(doc.get_line(r.end, true)) << std::endl;
#endif
      }
#ifdef DEBUG_GET_LIST_ELEMENTS
      std::cout << "  Trimmed end = " << r.end << " -- "
                << std::quoted(doc.get_line(r.end, true)) << std::endl;
#endif
      if (r.end < r.begin) {
        throw bizare_error()
            << "[ERROR]: After trimming, list item ends "
               "before beggining: begin = "
            << std::quoted(doc.get_line(r.begin, true))
            << ", end: " << std::quoted(doc.get_line(r.end, true));
      }
    }
  }
  return outV;
}

// #define DEBUG_OPEN_FHICL_FILE

inline std::unique_ptr<std::ifstream>
open_fhicl_file(std::string const &filename) {

#ifdef DEBUG_OPEN_FHICL_FILE
  std::cout << "[open_fhicl_file]: Trying to resolve " << std::quoted(filename)
            << std::endl;
#endif

  if (filename.find_last_of('/') !=
      std::string::npos) { // If there are slashes, assume that it is a
                           // relative path.
#ifdef DEBUG_OPEN_FHICL_FILE
    std::cout << "[open_fhicl_file]: Found a forward slash, assuming a "
                 "relative path."
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

inline fhicl_doc read_doc(std::string const &filename) {
  std::unique_ptr<std::ifstream> ifs = open_fhicl_file(filename);
  if (!ifs || !ifs->good()) {
    throw file_does_not_exist()
        << "[ERROR]: File: " << std::quoted(filename)
        << " could not be opened for reading. (N.B. files in the working "
           "directory must be explicitly qualified with \"./\" to avoid "
           "confusion with files found in FHICL_FILE_PATH)";
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
