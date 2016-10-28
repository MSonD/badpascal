#ifndef PAS_PARSER_H
#define PAS_PARSER_H
#include "parse_base.h"

//
class PascalParser : public Parser
{
  inline void E(Atom where, const std::string& ss);
  inline void T(Atom where, const std::string& ss);
  void TP(Atom where, const std::string& product, const std::string& ss);
  void N(Atom where, Atom nt,const std::string& ss);
  void NP(Atom where, const std::string& product, const std::string& ss);
  inline void pushElements(Atom,Entry, const std::string&);
  inline void pushElements(Atom,Entry, Atom);
  void TP(Atom where, const std::string& product);
  unsigned store(const std::string& str);
  void terminals();
public:
PascalParser();
};

#endif // PARSER_H
