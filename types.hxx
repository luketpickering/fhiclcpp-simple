#ifndef FHICLCPP_SIMPLE_TYPES_HXX_SEEN
#define FHICLCPP_SIMPLE_TYPES_HXX_SEEN

#include "utils.hxx"

#include <cctype>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>

namespace fhicl {

struct EInvalidKey : public std::exception {
  std::string msg;
  EInvalidKey(std::string const &m) { msg = m; }
  const char *what() const noexcept { return msg.c_str(); }
};
struct EValueIsNotLeaf : public std::exception {
  std::string msg;
  EValueIsNotLeaf(std::string const &m) { msg = m; }
  const char *what() const noexcept { return msg.c_str(); }
};
struct EFailedToParse : public std::exception {
  std::string msg;
  EFailedToParse(std::string const &m) { msg = m; }
  const char *what() const noexcept { return msg.c_str(); }
};

struct ECantInsert : public std::exception {
  std::string msg;
  ECantInsert(std::string const &m) { msg = m; }
  const char *what() const noexcept { return msg.c_str(); }
};

class ParameterSet;
template <typename T> T fhicl_get(ParameterSet const &ps, std::string const &k);
template <>
ParameterSet fhicl_get(ParameterSet const &ps, std::string const &k);
template <typename T>
void fhicl_put(ParameterSet &ps, std::string const &k, T const &v,
               bool can_replace = false);
template <>
void fhicl_put(ParameterSet &ps, std::string const &k, ParameterSet const &v,
               bool can_replace);

class ParameterSet {
  std::string Id;

  enum KeyType { kLeafKey, kBranchKey, kEither };
  std::map<std::string, ParameterSet> branches;
  std::map<std::string, std::string> leaves;

  std::shared_ptr<ParameterSet> Prolog;

  int get_sequence_dereference(std::string const &s) const {
    if (!is_valid_key(s)) {
      std::cout
          << "[ERROR]: Attempted to get sequence index from an invalid key: "
          << std::quoted(s) << std::endl;
      throw;
    }
    if (s.back() == ']') {
      size_t open_brace = s.find_first_of("[");
      int index = str2T<int>(
          s.substr(open_brace + 1, (s.size() - 1) - (open_brace + 1)));
      if (index < 0) {
        std::cout << "[ERROR]: Attempted to use sequence index: " << index
                  << ", but it must be a positive integer." << std::endl;
        throw;
      }
      return index;
    }
    return -1;
  }
  std::string strip_sequence_dereference(std::string const &s) const {
    if (!is_valid_key(s)) {
      std::cout
          << "[ERROR]: Attempted to strip sequence index from an invalid key: "
          << std::quoted(s) << std::endl;
      throw;
    }
    if (s.back() == ']') {
      size_t open_brace = s.find_first_of("[");
      std::cout << s.substr(0, open_brace) << std::endl;
      return s.substr(0, open_brace);
    } else {
      std::cout << "[ERROR]: Attempted to strip sequence index from an key "
                   "without a sequence index: "
                << std::quoted(s) << std::endl;
      throw;
    }
  }

  bool is_valid_key(std::string const &s) const {
    return s.size() && std::isalpha(s.front()) &&
           (s.find_first_of(" \":\'@(){}.") == std::string::npos) &&
           ((s.find_first_of("]") == std::string::npos) ||
            ((s.find_first_of("]") == (s.size() - 1))) &&
                (s.find_first_of("[") != std::string::npos));
  }

  ParameterSet &get_FQ_parent_branch(std::string const &s,
                                     bool allow_add = false) {
    std::vector<std::string> path = ParseToVect<std::string>(s, ".");
    ParameterSet *child_ps = this;
    for (size_t i = 0; i < (path.size() - 1); ++i) {
      if (!is_valid_key(path[i])) {
        std::cout << "[ERROR]: Attempted to follow invalid key "
                  << std::quoted(path[i]) << std::endl;
        throw;
      }
      if (child_ps->branches.find(path[i]) != child_ps->branches.end()) {
        child_ps = &child_ps->branches.find(path[i])->second;
      } else if (allow_add) {
        child_ps->branches.insert({path[i], ParameterSet()});
        child_ps = &child_ps->branches.find(path[i])->second;
      } else {
        std::cout << "[ERROR]: No such branch (\"" << path[i]
                  << "\") exists in ParameterSet: "
                  << child_ps->to_indented_string() << std::endl;
        throw;
      }
    }
    return *child_ps;
  }

  ParameterSet const &get_FQ_parent_branch(std::string const &s) const {
    std::vector<std::string> path = ParseToVect<std::string>(s, ".");
    ParameterSet const *child_ps = this;
    for (size_t i = 0; i < (path.size() - 1); ++i) {
      if (!is_valid_key(path[i])) {
        std::cout << "[ERROR]: Attempted to follow invalid key "
                  << std::quoted(path[i]) << std::endl;
        throw;
      }
      if (child_ps->branches.find(path[i]) != child_ps->branches.end()) {
        child_ps = &child_ps->branches.find(path[i])->second;
      } else {
        std::cout << "[ERROR]: No such branch (\"" << path[i]
                  << "\") exists in ParameterSet: "
                  << child_ps->to_indented_string() << std::endl;
        throw;
      }
    }
    return *child_ps;
  }

  bool leaf_is_sequence(std::string const &s) const {
    if (!has_FQ_key(s, kLeafKey)) {
      std::cout << "[ERROR]: Attempted to get whether key " << std::quoted(s)
                << " points to a sequence, but that key doesn't exist."
                << std::endl;
    }
    std::string const &val = get<std::string>(s);
    if ((val.front() == '[') && (val.back() == ']')) {
      return true;
    }
    return false;
  }
  bool has_FQ_key(std::string const &s, KeyType kt = kEither) const {
    if (!s.size()) {
      return false;
    }
    std::vector<std::string> path = ParseToVect<std::string>(s, ".");
    ParameterSet const *ps = this;
    for (size_t i = 0; i < (path.size() - 1); ++i) {
      if (ps->branches.find(path[i]) != ps->branches.end()) {
        ps = &ps->branches.find(path[i])->second;
      } else {
        return false;
      }
    }
    switch (kt) {
    case kLeafKey: {
      return (ps->leaves.find(path.back()) != ps->leaves.end());
    }
    case kBranchKey: {
      return (ps->branches.find(path.back()) != ps->branches.end());
    }
    default: {
      return (ps->leaves.find(path.back()) != ps->leaves.end()) ||
             (ps->branches.find(path.back()) != ps->branches.end());
    }
    }
  }

  template <typename T>
  friend T fhicl_get(ParameterSet const &, std::string const &);
  friend ParameterSet fhicl_get(ParameterSet const &, std::string const &);

  template <typename T>
  friend void fhicl_put(ParameterSet &, std::string const &, T const &, bool);
  friend void fhicl_put(ParameterSet &, std::string const &,
                        ParameterSet const &, bool);

  std::string to_string(std::string indent, bool doIndent) const {
    std::stringstream ss("");
    if (doIndent) {
      indent += "  ";
    }
    ss << "{";
    if (doIndent) {
      ss << std::endl;
    }
    if (Prolog) {
      ss << "PROLOG: " << Prolog->to_string(indent, doIndent);
      if (doIndent) {
        ss << std::endl;
      }
    }
    for (auto const &branch : branches) {
      if (doIndent) {
        ss << indent;
      }
      ss << branch.first << ": "
         << branch.second.to_string(indent + "  ", doIndent);
      if (doIndent) {
        ss << std::endl;
      }
    }
    for (auto const &leaf : leaves) {
      if (doIndent) {
        ss << indent;
      }
      ss << leaf.first << ": " << leaf.second;
      if (doIndent) {
        ss << std::endl;
      }
    }
    if (doIndent) {
      indent = indent.substr(0, indent.size() - 2);
      ss << indent;
    }
    ss << "{";
    if (doIndent) {
      ss << std::endl;
    }
    return ss.str();
  }

public:
  ParameterSet() : Prolog(nullptr) {}

  bool has_key(std::string const &s) const { return has_FQ_key(s); }

  template <typename T> T get(std::string const &k) const {
    return fhicl_get<T>(*this, k);
  }
  template <typename T>
  void put(std::string const &k, T const &v, bool AddToProlog = false) {
    if (AddToProlog && !Prolog) {
      Prolog = std::make_shared<ParameterSet>();
    }
    return fhicl_put<T>(AddToProlog ? (*this->Prolog) : (*this), k, v);
  }
  template <typename T>
  void put_or_replace(std::string const &k, T const &v,
                      bool AddToProlog = false) {
    if (AddToProlog && !Prolog) {
      Prolog = std::make_shared<ParameterSet>();
    }
    return fhicl_put<T>(AddToProlog ? (*this->Prolog) : (*this), k, v, true);
  }

  std::string to_string() const { return to_string("", false); }
  std::string to_indented_string() const { return to_string("", true); }
};

template <typename T>
T fhicl_get(ParameterSet const &ps, std::string const &k) {
  if (!ps.has_FQ_key(k)) {
    std::stringstream ss("");
    ss << "Couldn't find key: " << std::quoted(k);
    throw EInvalidKey(ss.str());
  }
  if (!ps.has_FQ_key(k, ParameterSet::kLeafKey)) {
    throw EValueIsNotLeaf("Attempted to parse a fhicl table as a leaf type.");
  }
  ParameterSet const &child_ps = ps.get_FQ_parent_branch(k);
  std::string const &leaf_key = ParseToVect<std::string>(k, ".").back();
  if (child_ps.leaves.find(leaf_key) != child_ps.leaves.end()) {
    return str2T<T>(child_ps.leaves.find(leaf_key)->second);
  } else {
    std::cout << "[ERROR]: No such leaf (\"" << leaf_key
              << "\") exists in ParameterSet: " << child_ps.to_indented_string()
              << std::endl;
    throw;
  }
}

template <>
ParameterSet fhicl_get(ParameterSet const &ps, std::string const &k) {
  if (!ps.has_FQ_key(k)) {
    std::stringstream ss("");
    ss << "Couldn't find key: " << std::quoted(k);
    throw EInvalidKey(ss.str());
  }
  if (!ps.has_FQ_key(k, ParameterSet::kBranchKey)) {
    throw EValueIsNotLeaf("Attempted to parse a leaf as a fhicl table.");
  }
  ParameterSet const &child_ps = ps.get_FQ_parent_branch(k);
  std::string const &branch_key = ParseToVect<std::string>(k, ".").back();
  if (child_ps.branches.find(branch_key) != child_ps.branches.end()) {
    return child_ps.branches.find(branch_key)->second;
  } else {
    std::cout << "[ERROR]: No such branch (\"" << branch_key
              << "\") exists in ParameterSet: " << child_ps.to_indented_string()
              << std::endl;
    throw;
  }
}

template <typename T>
void fhicl_put(ParameterSet &ps, std::string const &k, T const &v,
               bool can_replace) {
  if (!can_replace && ps.has_FQ_key(k)) {
    std::stringstream ss("");
    ss << "Cannot overwrite key: " << std::quoted(k);
    throw ECantInsert(ss.str());
  }
  ParameterSet &child_ps = ps.get_FQ_parent_branch(k, true);
  auto bla = ParseToVect<std::string>(k, ".");
  std::string leaf_key = bla.back();
  // Names must be unique, if we are overwriting a branch-type, make sure to
  // remove it.
  if (ps.has_FQ_key(k, ParameterSet::kBranchKey)) {
    child_ps.branches.erase(leaf_key);
  }
  int seq_idx = ps.get_sequence_dereference(leaf_key);
  if (seq_idx != -1) {
    leaf_key = ps.strip_sequence_dereference(leaf_key);
    std::vector<T> seq;
    if (child_ps.leaves.find(leaf_key) != child_ps.leaves.end()) {
      seq = str2T<std::vector<T>>(child_ps.leaves[leaf_key]);
    }
    if (seq.size() <= seq_idx) {
      seq.resize(seq_idx + 1);
    }
    seq[seq_idx] = T2Str<T>(v);
    child_ps.leaves[leaf_key] = T2Str<std::vector<T>>(seq);
  } else {
    child_ps.leaves[leaf_key] = T2Str<T>(v);
  }
}

template <>
void fhicl_put(ParameterSet &ps, std::string const &k, ParameterSet const &v,
               bool can_replace) {
  if (!can_replace && ps.has_FQ_key(k)) {
    std::stringstream ss("");
    ss << "Cannot overwrite key: " << std::quoted(k);
    throw ECantInsert(ss.str());
  }
  ParameterSet &child_ps = ps.get_FQ_parent_branch(k, true);
  auto bla = ParseToVect<std::string>(k, ".");
  std::string const &branch_key = bla.back();
  // Names must be unique, if we are overwriting a branch-type, make sure to
  // remove it.
  if (ps.has_FQ_key(k, ParameterSet::kLeafKey)) {
    child_ps.leaves.erase(branch_key);
  }
  int seq_idx = ps.get_sequence_dereference(branch_key);
  if (seq_idx != -1) {
    std::cout << "[ERROR]: Attempted to add a fhicl table to a sequence (\""
              << branch_key << "\")." << std::endl;
    throw;
  }
  child_ps.branches[branch_key] = v;
}

} // namespace fhicl

#endif
