#ifndef PAS_BASEPARSE_H
#define PAS_BASEPARSE_H
#include <initializer_list>
#include <string>
#include "data.h"
#include "lexer_base.h"
struct Entry{
  unsigned char flags = 0;
  unsigned char replace = 0;
  enum flags: unsigned char{
    POP = 1,
    STOP = 2,
    FINISH = 4,
  };
};

const std::string S_nsymbols [] = {
  "M",
  "V",
  "H",
  "D'",
  "J",
  "G",
  "S",
  "A",
  "L",
  "W",
  "U",
  "Y",
  "X",
  "K",
  "P",
  "B",
  "I",
  "O",
  "R",
  "Q",
  "D",
  "Z",
  "E",
  "E'",
  "T",
  "T'",
  "F",
  "C",
  "R'"
};

class AtomSource{
public:
  virtual Atom fetch() = 0;
};

class Parser
{
  AtomSource* source;
  using E = Entry;
  Vector<Entry> table;
  Vector<std::string> replace_strings;
  Vector<Atom> stack;
  size_t width;
  Atom head;
  void pushString(const std::string&);
public:
  void setSource(AtomSource& src);
  void step();
};

#endif // PARSE_H
