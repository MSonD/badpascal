#ifndef PAS_BASEPARSE_H
#define PAS_BASEPARSE_H
#include <initializer_list>
#include <string>
#include <iostream>
#include "assert.h"
#include "data.h"
#include "lexer_base.h"
template <class T>
void printString(T str){
  for(auto& s : str){
    std::cout << S_atoms[s];
  }
}
//Tipo del elemento de la tabla de parser
struct Entry{
public:
  enum : unsigned char{
    POP = 1,
    STOP = 2,
    REPLACE = 4,
    FORWARD = 8,
    FINISH = 16,
  };
  unsigned char flags = 0;
  unsigned char replace = 0;
  Entry(unsigned char, unsigned char);
  Entry();
};

const std::string S_nsymbols [] = {
  "M",
  "N",
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
//Un origen del cual se obtienen átomos
class AtomSource{
public:
  virtual Atom fetch() = 0;
  virtual unsigned line() = 0;
};

class Parser
{
protected:
  AtomSource* source;
  using E = Entry;
  Vector<Entry> table;
  Vector<std::string> replace_strings;
  Vector<Atom> stack;
  size_t width;
  Atom head;
  bool failure = false;
  void pushString(const std::string&);
public:
  void setSource(AtomSource& src);
  bool step();
  bool operator()();
};

#endif // PARSE_H
