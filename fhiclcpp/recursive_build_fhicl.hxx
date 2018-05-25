#ifndef FHICLCPP_SIMPLE_FHICLCPP_RECURSIVE_BUILD_FHICL_H_SEEN
#define FHICLCPP_SIMPLE_FHICLCPP_RECURSIVE_BUILD_FHICL_H_SEEN

#include "fhiclcpp/ParameterSet.h"

#include "fhiclcpp/fhicl_doc.hxx"

#include <cstdio>

namespace fhicl {

#define FHICLCPP_SIMPLE_PARSERS_DEBUG

#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
std::string indent = "";
#endif

ParameterSet recursive_build_fhicl(
    fhicl_doc &doc, ParameterSet const &working_set = ParameterSet(),
    ParameterSet const &PROLOG = ParameterSet(),
    fhicl_doc_line_point begin = {0, 0},
    fhicl_doc_line_point end = {std::numeric_limits<size_t>::max(),
                                std::string::npos},
    bool in_prolog = false, key_t const &current_key = "") {

  ParameterSet ps;

  fhicl_doc_line_point read_ptr = doc.find_first_not_of(" ", begin);
  end = doc.validate_line_point(end);
  // std::cout << doc.to_string(true, true) << std::endl;

  while (read_ptr < end) {

    fhicl_doc_line_point next_char = doc.find_first_of(" ", read_ptr, false);

    if ((doc.get_char(read_ptr) == '#') ||
        (doc.substr(read_ptr, doc.advance(read_ptr, 2)) == "//")) {
#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
      std::cout << indent << "Found comment at: " << read_ptr << " "
                << std::quoted(doc.get_line(read_ptr)) << " from "
                << doc.get_line_info(read_ptr);
#endif
      // move to the next line
      read_ptr = doc.advance_line(next_char);
      continue;
    }

    std::string token = doc.substr(read_ptr, next_char);

#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
    std::cout << indent << "Reading: (" << read_ptr << " -- " << next_char
              << ") = " << token << std::endl;
#endif

    if (token == "BEGIN_PROLOG") {
      in_prolog = true;
#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
      std::cout << indent << "[INFO]: In prolog." << std::endl;
#endif
    } else if (token == "END_PROLOG") {
      in_prolog = false;
#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
      std::cout << indent << "[INFO]: No longer in prolog." << std::endl;
#endif
    } else {
      if (token.back() != ':') {
        throw malformed_document()
            << "[ERROR]: Expected a key declaraction like \"key: \", but "
               "instead found "
            << std::quoted(token) << " from " << doc.get_line_info(read_ptr);
      }
      key_t key = token.substr(0, token.size() - 1);

      // respond based on value type:
      // cannot be a line break after the colon
      fhicl_doc_line_point next_not_space =
          doc.find_first_not_of(" ", doc.advance(next_char), false);
      if (next_not_space.isendofline()) {
        throw malformed_document()
            << "[ERROR]: When search for value to key defined on line: "
            << doc.get_line(next_char) << " at " << doc.get_line_info(next_char)
            << " failed to find value on same line.";
      }
      switch (doc.get_char(next_not_space)) {
      case '{': {
        // table member
        next_char = find_matching_bracket(doc, '{', '}', next_not_space);
#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
        std::cout << indent << "[INFO]: Found table KV: {" << std::quoted(key)
                  << ": {"
                  << std::quoted(
                         doc.substr(doc.advance(next_not_space), next_char))
                  << "} } at " << doc.get_line_info(next_not_space)
                  << std::endl;
        indent += "  ";
#endif
        ParameterSet table = recursive_build_fhicl(
            doc, working_set, PROLOG, doc.advance(next_not_space), next_char,
            in_prolog, current_key);
        ps.put_with_custom_history(key, table,
                                   doc.get_line_info(next_not_space));
        next_char = doc.advance(next_char);
#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
        indent = indent.substr(2);
#endif
        break;
      }
      case '[': {
        // list member
        next_char = find_matching_bracket(doc, '[', ']', next_not_space);
        // std::string value =
        //     fhicl.substr(next_not_space, (next_char + 1) - next_not_space);
        // ps.put_or_replace(key, value);
        next_char = doc.advance(next_char);
#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
        std::cout << indent << "[INFO]: Found sequence KV: {"
                  << std::quoted(key) << ": ["
                  << std::quoted(
                         doc.substr(doc.advance(next_not_space), next_char))
                  << "]}. at " << doc.get_line_info(next_not_space)
                  << std::endl;
#endif
        break;
      }
      case '\"': {
        // string member
        next_char = doc.find_first_of("\"", doc.advance(next_not_space), false);

        if (next_char.isendofline()) {
          throw malformed_document()
              << "[ERROR]: Failed to find matching quote to: " << next_not_space
              << " from " << doc.get_line_info(next_not_space);
        }
        std::string value = doc.substr(doc.advance(next_not_space), next_char);
        ps.put_with_custom_history(key, value,
                                   doc.get_line_info(next_not_space));
        next_char = doc.advance(next_char);
#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
        std::cout << indent << "[INFO]: Found KV: {" << std::quoted(key) << ":"
                  << std::quoted(value) << "}. at "
                  << doc.get_line_info(next_not_space) << std::endl;
#endif
        break;
      }
      case '@': {
        next_char = doc.find_first_of(" :", doc.advance(next_not_space), false);
        std::string directive =
            doc.substr(doc.advance(next_not_space), next_char);

        if (!next_char.isend()) {
          if (directive == "nil") {
            ps.put_with_custom_history(key, "@nil",
                                       doc.get_line_info(next_not_space));
#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
            std::cout << indent << "[INFO]: Found nil directive at "
                      << doc.get_line_info(next_not_space) << std::endl;
#endif
          } else {
            if (doc.get_char(doc.advance(next_char)) != ':') {
              throw malformed_document()
                  << "[ERROR]: Expected to find double colon separator between "
                     "@directive and the key name argument on line: "
                  << doc.get_line(next_char)
                  << " at: " << doc.advance(next_char)
                  << ", but instead found: "
                  << doc.get_char(doc.advance(next_char));
            }
            fhicl_doc_line_point next_space =
                doc.find_first_of(" ", doc.advance(next_char, 2), false);

            key_t directive_key =
                doc.substr(doc.advance(next_char, 2), next_space);
            if (directive == "local") {
#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
              std::cout << indent << "[INFO]: Found local directive: "
                        << std::quoted(directive_key) << " at "
                        << doc.get_line_info(next_not_space) << std::endl;
#endif
            } else if (directive == "table") {
#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
              std::cout << indent << "[INFO]: Found table directive: "
                        << std::quoted(directive_key) << " at "
                        << doc.get_line_info(next_not_space) << std::endl;
#endif
            } else if (directive == "sequence") {
#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
              std::cout << indent << "[INFO]: Found sequence directive: "
                        << std::quoted(directive_key) << " at "
                        << doc.get_line_info(next_not_space) << std::endl;
#endif
            } else {
              throw malformed_document()
                  << "[ERROR]: Unknown fhicl directive: "
                  << std::quoted(directive)
                  << ", expecting one of \"nil\", \"local\", \"table\", or "
                     "\"sequence\". at "
                  << doc.get_line_info(read_ptr);
            }

            next_char = next_space;
          }
        } else {
          throw malformed_document()
              << "[ERROR]: Found incomplete fhicl directive: "
              << std::quoted(doc.get_line_info(read_ptr))
              << ", expecting one of \"nil\", \"local\", \"table\", or "
                 "\"sequence\". at "
              << doc.get_line_info(read_ptr);
        }
        break;
      }
      default: { //
        next_char = doc.find_first_of(" ", next_not_space, false);
        std::string value = doc.substr(next_not_space, next_char);

        ps.put_with_custom_history(key, value,
                                   doc.get_line_info(next_not_space));
#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
        std::cout << indent << "[INFO]: Found KV: " << std::quoted(key) << ": "
                  << std::quoted(value) << ". at "
                  << doc.get_line_info(next_char) << std::endl;
#endif
      }
      }
    }
#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
    std::cout << indent << "After reading value, next_char = " << next_char
              << " = \'" << doc.get_char(next_char)
              << "\' from line: " << std::quoted(doc.get_line(next_char)) << "."
              << std::endl;
#endif
    if (!next_char.isend()) {
      read_ptr = doc.find_first_not_of(" ", next_char);
    } else {
      read_ptr.setend();
    }
#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
    std::cout << indent << "Next to read: " << read_ptr << " = \'"
              << doc.get_char(read_ptr)
              << "\' from line: " << std::quoted(doc.get_line(read_ptr)) << "."
              << std::endl;
#endif
  }

  return ps;
} // namespace fhicl

} // namespace fhicl

#endif
