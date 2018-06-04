#ifndef FHICLCPP_SIMPLE_FHICLCPP_RECURSIVE_BUILD_FHICL_H_SEEN
#define FHICLCPP_SIMPLE_FHICLCPP_RECURSIVE_BUILD_FHICL_H_SEEN

#include "fhiclcpp/ParameterSet.h"

#include "fhiclcpp/fhicl_doc.hxx"

#include <cstdio>

namespace fhicl {

inline ParameterSet parse_fhicl_document(fhicl_doc const &, ParameterSet const &,
                                  ParameterSet const &, fhicl_doc_range,
                                  key_t const &);

// #define FHICLCPP_SIMPLE_PARSERS_DEBUG

#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
std::string indent = "";
#endif

template <typename T>
inline std::shared_ptr<T>
deep_copy_resolved_reference_value(key_t const &key,
                                   ParameterSet const &working_set,
                                   ParameterSet const &PROLOG) {

  std::shared_ptr<Base> base_val = working_set.get_value_recursive(key);
  std::shared_ptr<Base> PROLOG_val = PROLOG.get_value_recursive(key);

  if ((!base_val) && (!PROLOG_val)) {
    std::stringstream ss("");
    ss << std::endl
       << "\t PROLOG: { " << PROLOG.to_string() << "}" << std::endl
       << "\t working_set: { " << working_set.to_string() << "}" << std::endl;
    throw nonexistant_key()
        << "[ERROR]: Failed to resolve reference directive as key: "
        << std::quoted(key)
        << " cannot be found. N.B. Reference keys must be fully qualified. "
           "\nCurrent document:"
        << ss.str();
  }

  // Non-PROLOG takes precedence
  std::shared_ptr<T> value_for_ref =
      std::dynamic_pointer_cast<T>(working_set.get_value_recursive(key));
  if (value_for_ref) {
    return std::dynamic_pointer_cast<T>(deep_copy_value(value_for_ref));
  } else if (base_val) {
    throw wrong_fhicl_category()
        << "[ERROR]: Attempted to resolve reference to key: "
        << std::quoted(key)
        << " as fhicl category: " << fhicl_type<T>::category_string()
        << " but resolved key is of type: "
        << working_set.get_fhicl_category_string(key);
  }

  std::shared_ptr<T> PROLOG_value_for_ref =
      std::dynamic_pointer_cast<T>(PROLOG.get_value_recursive(key));
  if (PROLOG_value_for_ref) {
    return std::dynamic_pointer_cast<T>(deep_copy_value(PROLOG_value_for_ref));
  } else if (PROLOG_val) {
    throw wrong_fhicl_category()
        << "[ERROR]: Attempted to resolve reference to key: "
        << std::quoted(key)
        << " as fhicl category: " << fhicl_type<T>::category_string()
        << " but resolved key is of type: "
        << PROLOG.get_fhicl_category_string(key);
  }

  return nullptr;
}

inline std::shared_ptr<Base>
parse_object(fhicl_doc const &doc, fhicl_doc_range doc_range,
             fhicl_doc_line_point &next_character,
             ParameterSet const &working_set, ParameterSet const &PROLOG,
             key_t const &current_key, bool build_sequence = false) {

#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
  if (!doc_range.end.isend()) {
    std::cout << indent
              << "[INFO]: Parsing first object between: " << doc_range.begin
              << " = " << std::quoted(doc.get_line(doc_range.begin, true))
              << " and " << doc_range.end << " = "
              << std::quoted(doc.get_line(doc_range.end, true)) << std::endl;
  } else {
    std::cout << indent
              << "[INFO]: Parsing first object from: " << doc_range.begin
              << " = " << std::quoted(doc.get_line(doc_range.begin, true))
              << std::endl;
  }
#endif

  fhicl_doc_line_point next_not_space =
      doc.find_first_not_of(" ", doc_range.begin, false);

#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
  std::cout << indent << "[INFO]: Next not space found at: "
            << std::quoted(doc.get_line(next_not_space, true)) << std::endl;
#endif

  if (next_not_space.isendofline()) {
    throw malformed_document()
        << "[ERROR]: When searching for value to key defined on line: "
        << std::quoted(doc.get_line(doc_range.begin, true)) << " at "
        << std::quoted(doc.get_line_info(doc_range.begin))
        << " failed to find value on same line.";
  }
  switch (doc.get_char(next_not_space)) {
  case '{': {
    // table member
    next_character = find_matching_bracket(doc, '{', '}', next_not_space);
#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
    std::cout << indent << "[INFO]: Found table KV: {"
              << std::quoted(current_key) << ": {"
              << std::quoted(
                     doc.substr(doc.advance(next_not_space), next_character))
              << "} } at " << doc.get_line_info(next_not_space) << std::endl;
    indent += "  ";
#endif
    std::shared_ptr<ParameterSet> table =
        std::make_shared<ParameterSet>(std::move(parse_fhicl_document(
            doc, working_set, PROLOG,
            {doc.advance(next_not_space), next_character}, current_key)));
    next_character = doc.advance(next_character);
#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
    indent = indent.substr(2);
#endif
    return table;
  }
  case '[': {
    // list member
    fhicl_doc_line_point seq_end =
        find_matching_bracket(doc, '[', ']', next_not_space);

#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
    std::cout << indent << "[INFO]: Found sequence object: "
              << std::quoted(doc.substr(next_not_space, doc.advance(seq_end)))
              << ". at " << std::quoted(doc.get_line(next_not_space, true))
              << " from " << doc.get_line_info(next_not_space) << std::endl;
#endif

    std::shared_ptr<Sequence> seq = std::make_shared<Sequence>();
    auto const &seq_element_str_reps =
        get_list_elements(doc, {doc.advance(next_not_space), seq_end}, true);

    for (size_t el_it = 0; el_it < seq_element_str_reps.size(); ++el_it) {
      fhicl_doc_range el_range = seq_element_str_reps[el_it];
      fhicl_doc_line_point last_parsed_char;
#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
      std::cout << indent << " -- Sequence element between "
                << std::quoted(doc.get_line(el_range.begin, true)) << " and "
                << std::quoted(doc.get_line(el_range.end, true)) << " = "
                << std::quoted(doc.substr(el_range)) << std::endl;
      indent += "  ";
#endif
      std::shared_ptr<Base> el_obj =
          parse_object(doc, el_range, last_parsed_char, working_set, PROLOG,
                       current_key, true);
#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
      indent = indent.substr(2);
#endif
      std::string unused_chars = doc.substr(last_parsed_char, el_range.end);
      string_parsers::trim(unused_chars);
      if (unused_chars.size()) {
        throw malformed_document()
            << "[ERROR]: When parsing sequence, started at " << el_range.begin
            << " on line " << std::quoted(doc.get_line(el_range.begin, true))
            << " from " << std::quoted(doc.get_line_info(el_range.begin))
            << " failed to parse: " << std::quoted(unused_chars) << " on line "
            << std::quoted(doc.get_line(last_parsed_char, true)) << " from "
            << std::quoted(doc.get_line_info(last_parsed_char))
            << " was there a newline in the middle of an atom element?";
      }

#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
      std::cout << indent << "[INFO]: Sequence element[" << el_it
                << "]: " << el_obj->to_string() << " read from "
                << std::quoted(doc.get_line(el_range.begin, true)) << " to "
                << std::quoted(doc.get_line(last_parsed_char, true))
                << std::endl;
#endif
      // Handle the result of @sequence directives.
      std::shared_ptr<Sequence> child_seq =
          std::dynamic_pointer_cast<Sequence>(el_obj);
      if (child_seq) { // is sequence directive, splice
        std::string str_rep = doc.substr(el_range);
        string_parsers::trim(str_rep);
        if (str_rep.find("@sequence") == 0) {
          seq->splice(std::move(*child_seq));
        } else {
          seq->put(std::move(el_obj));
        }
      } else {
        seq->put(std::move(el_obj));
      }
    }
#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
    std::cout << indent << "[INFO]: Parsed sequence = " << seq->to_string()
              << std::endl;
#endif
    next_character = doc.advance(seq_end);
    return seq;
  }
  case '\"': {
    // string member
    next_character =
        doc.find_first_of("\"", doc.advance(next_not_space), false);

    if (next_character.isendofline()) {
      throw malformed_document()
          << "[ERROR]: Failed to find matching quote to: "
          << std::quoted(doc.get_line(next_not_space, true)) << " from "
          << std::quoted(doc.get_line_info(next_not_space))
          << ". N.B. quoted strings cannot span multiple lines.";
    }

    std::string value = doc.substr(doc.advance(next_not_space), next_character);
    next_character = doc.advance(next_character);
#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
    std::cout << indent << "[INFO]: Found KV: {" << std::quoted(current_key)
              << ":" << std::quoted(value) << "}. at "
              << doc.get_line_info(next_not_space) << std::endl;
#endif
    return std::make_shared<Atom>(value);
  }
  case '@': {
    next_character =
        doc.find_first_of(" :", doc.advance(next_not_space), false);
    std::string directive =
        doc.substr(doc.advance(next_not_space), next_character);

    if (!next_character.isend()) {
      if (directive == "nil") {
#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
        std::cout << indent << "[INFO]: Found nil directive at "
                  << doc.get_line_info(next_not_space) << std::endl;
#endif
        return std::make_shared<Atom>("@nil");
      } else {
        if (doc.get_char(doc.advance(next_character)) != ':') {
          throw malformed_document()
              << "[ERROR]: Expected to find double colon separator between "
                 "@directive and the key name argument on line: "
              << std::quoted(doc.get_line(next_character, true))
              << " at: " << doc.advance(next_character)
              << ", but instead found: "
              << doc.get_char(doc.advance(next_character));
        }
        fhicl_doc_line_point end_of_directive = doc.advance(next_character, 2);
        next_character = doc.find_first_of(" ", end_of_directive, false);

        if (next_character > doc_range.end) {
          next_character = doc_range.end;
        }

        key_t directive_key = doc.substr(end_of_directive, next_character);
        if (directive == "local") {
#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
          std::cout << indent << "[INFO]: Found local directive: "
                    << std::quoted(directive_key) << " at "
                    << doc.get_line_info(next_not_space) << std::endl;
#endif
          return deep_copy_resolved_reference_value<Base>(directive_key,
                                                          working_set, PROLOG);
        } else if (directive == "table") {
          throw malformed_document()
              << "[ERROR]: Found @table directive "
              << (build_sequence ? " in sequence " : " with key ")
              << " but it should only be used directly within a table body to "
                 "splice in a referenced table. Found at "
              << std::quoted(doc.get_line(next_not_space, true)) << " from "
              << std::quoted(doc.get_line_info(next_not_space));
        } else if (directive == "sequence") {

          if (!build_sequence) {
            throw malformed_document()
                << "[ERROR]: Found @sequence directive outside of a sequence. "
                   "It can only be used within a sequence definition to splice "
                   "in a reference sequence. Found at "
                << std::quoted(doc.get_line(next_not_space, true)) << " from "
                << std::quoted(doc.get_line_info(next_not_space));
          }
#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
          std::cout << indent << "[INFO]: Found sequence directive: "
                    << std::quoted(directive_key) << " at "
                    << doc.get_line_info(next_not_space) << std::endl;
#endif
          return deep_copy_resolved_reference_value<Sequence>(
              directive_key, working_set, PROLOG);
        } else {
          throw malformed_document()
              << "[ERROR]: Unknown fhicl directive: " << std::quoted(directive)
              << ", expecting one of \"nil\", \"local\", \"table\", or "
                 "\"sequence\". Found at "
              << std::quoted(doc.get_line(next_not_space, true)) << " from "
              << std::quoted(doc.get_line_info(next_not_space));
        }
      }
    } else {
      throw malformed_document()
          << "[ERROR]: Found incomplete fhicl directive beginning: "
          << std::quoted(doc.get_line(next_not_space, true))
          << ", expecting one of \"nil\", \"local\", \"table\", or "
             "\"sequence\". From document: "
          << std::quoted(doc.get_line_info(next_not_space));
    }
  }
  default: { // simple atom type
    next_character = doc.find_first_of(" ", next_not_space, false);
    if (next_character > doc_range.end) {
      next_character = doc_range.end;
    }
    std::string value = doc.substr(next_not_space, next_character);
#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
    std::cout << indent << "[INFO]: Found KV: " << std::quoted(current_key)
              << ": " << std::quoted(value) << ". at "
              << doc.get_line_info(next_character) << std::endl;
#endif
    return std::make_shared<Atom>(value);
  }
  }
}

ParameterSet
parse_fhicl_document(fhicl_doc const &doc,
                     ParameterSet const &_working_set = ParameterSet(),
                     ParameterSet const &_PROLOG = ParameterSet(),
                     fhicl_doc_range range = fhicl_doc_range::all(),
                     key_t const &current_key = "") {

  bool in_prolog = false;
  std::shared_ptr<ParameterSet> PROLOG =
      std::make_shared<ParameterSet>(_PROLOG);
  std::shared_ptr<ParameterSet> working_set =
      std::make_shared<ParameterSet>(_working_set);
  std::shared_ptr<ParameterSet> ps;
  if (current_key
          .size()) { // if we have recursed into a child, the working set will
                     // contain information and the currently building parameter
                     // set should be correctly placed within it
    working_set->put(current_key, ParameterSet());
    ps = std::dynamic_pointer_cast<ParameterSet>(
        working_set->get_value_recursive(current_key));
    if (!ps) {
      throw internal_error()
          << "[ERROR]: When attempting to add working ParameterSet to "
             "working_set.";
    }
  } else { // if this is the top level, then we don't really care and
           // _working_set should be empty.
    ps = working_set;
  }

  fhicl_doc_line_point read_ptr = doc.find_first_not_of(" ", range.begin);
  range.end = doc.validate_line_point(range.end);

  while (read_ptr < range.end) {

    fhicl_doc_line_point next_char = doc.find_first_of(" ", read_ptr, false);

    if ((doc.get_char(read_ptr) == '#') ||
        (doc.substr(read_ptr, doc.advance(read_ptr, 2)) == "//")) {
#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
      std::cout << indent << "Found comment at: " << read_ptr << " "
                << std::quoted(doc.get_line(read_ptr, true)) << " from "
                << doc.get_line_info(read_ptr) << std::endl;
#endif
      // move to the next line
      read_ptr = doc.advance_line(next_char);
      continue;
    }

    std::string token = doc.substr(read_ptr, next_char);

#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
    std::cout << indent << "Reading: ("
              << std::quoted(doc.get_line(read_ptr, true)) << " -- "
              << std::quoted(doc.get_line(next_char, true))
              << ") = " << std::quoted(token) << std::endl;
#endif

    // A few special cases.
    if (token == "BEGIN_PROLOG") {
      if (working_set->get_names().size() || ps->get_names().size()) {
        std::stringstream ss("");
        for (auto const &name : working_set->get_names()) {
          ss << name << ": " << working_set->get_src_info(name) << std::endl;
        }
        throw malformed_document()
            << "[ERROR]: Found BEGIN_PROLOG directive at "
            << doc.get_line_info(read_ptr)
            << " after non-prolog key: value pairs have been defined: "
            << ss.str();
      }
      in_prolog = true;
#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
      std::cout << indent << "[INFO]: In prolog." << std::endl;
#endif
    } else if (token == "END_PROLOG") {
      in_prolog = false;
#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
      std::cout << indent
                << "[INFO]: No longer in prolog: " << PROLOG->to_string()
                << std::endl;
#endif
    } else if (token.front() == '@') {
      if (token.substr(0, 8) ==
          "@table::") { // @table:: is the only directive that is allowed
        // to be keyless and may only appear within a table context

        fhicl_doc_line_point key_start = doc.advance(read_ptr, 8);
        fhicl_doc_line_point next_space =
            doc.find_first_of(" ", key_start, false);

        key_t table_key = doc.substr(key_start, next_space);

#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
        std::cout << indent
                  << "[INFO]: Found table directive: " << std::quoted(table_key)
                  << " at " << doc.get_line_info(key_start) << std::endl;
#endif

        std::shared_ptr<ParameterSet> table_for_splice =
            deep_copy_resolved_reference_value<ParameterSet>(
                table_key, *working_set, *PROLOG);
        (in_prolog ? PROLOG : ps)->splice(std::move(*table_for_splice));
      } else {
        throw malformed_document()
            << "[ERROR]: Found keyless fhicl directive: " << std::quoted(token)
            << " on line : " << std::quoted(doc.get_line(read_ptr, true))
            << ", but only the \"@table\" directive is allowed to appear "
               "without a key. From "
            << doc.get_line_info(read_ptr);
      }
    } else {
      if (token.back() != ':') {
        throw malformed_document()
            << "[ERROR]: Expected a key declaraction like \"key: \", but "
               "instead found "
            << std::quoted(token) << " at "
            << std::quoted(doc.get_line(read_ptr, true)) << " from "
            << doc.get_line_info(read_ptr);
      }
      key_t key = token.substr(0, token.size() - 1);

      std::string new_object_key =
          (current_key.size() ? (current_key + ".") : current_key) + key;

#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
      std::cout << indent << "Parsing next object from " << next_char << " = "
                << std::quoted(doc.get_line(next_char, true)) << std::endl;
#endif

      std::shared_ptr<Base> new_obj =
          parse_object(doc, {next_char, fhicl_doc_line_point::end()}, next_char,
                       *working_set, *PROLOG, new_object_key);

      (in_prolog ? PROLOG : ps)
          ->put_with_custom_history(key, std::move(new_obj),
                                    doc.get_line_info(read_ptr));
    }
#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
    std::cout << indent << "After reading value, next_char = " << next_char
              << " = \'" << doc.get_char(next_char)
              << "\' from line: " << std::quoted(doc.get_line(next_char, true))
              << "." << std::endl;
#endif
    if (!next_char.isend()) {
      read_ptr = doc.find_first_not_of(" ", next_char);
    } else {
      read_ptr = fhicl_doc_line_point::end();
    }
#ifdef FHICLCPP_SIMPLE_PARSERS_DEBUG
    std::cout << indent << "Next to read: " << read_ptr << " = \'"
              << doc.get_char(read_ptr)
              << "\' from line: " << std::quoted(doc.get_line(read_ptr, true))
              << "." << std::endl;
#endif
  }

  return (*ps);
}
} // namespace fhicl

#endif
