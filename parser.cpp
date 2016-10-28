#include "include/parser.h"
#include <iostream>
PascalParser::PascalParser()
{
  table.resize(static_cast<unsigned>(I_INDEXES::END) * (static_cast<unsigned>(I_INDEXES::END) + sizeof_a(S_nsymbols)));
  stack.push_back(56);
  width = static_cast<unsigned>(I_INDEXES::END);
  head = 56;
  terminals();
  //M
  TP(0x38,"\x14\x2b\x14\x3b\x2c\x30\x39\x32","\x0d");
  //N
  NP(0x39,"\x3a\x46","\x12");
  N(0x39,0x46,"\x02");
  //V
  TP(0x3A,"\x14\x3b\x31\x3c\x30\x3e","\x12");
  //H
  TP(0x3B,"\x14\x3b","\x2f");
  E(0x3B,"\x2c\x31");
  //D'
  TP(0x3C,"\x3d","\x17");
  TP(0x3C,"\x3d","\x18");
  TP(0x3C,"\x3d","\x15");
  TP(0x3C,"\x3d","\x16");
  //J
  
}

inline void PascalParser::pushElements(Atom where, Entry e, const std::string& ss)
{
  for(unsigned char i : ss)
    table[where*width+i] = e;
}

inline void PascalParser::E(Atom where, const std::string& ss)
{
  pushElements(where,{Entry::POP|Entry::STOP,0},ss);
}

inline void PascalParser::T(Atom where, const std::__cxx11::string& ss)
{
  pushElements(where,{Entry::POP|Entry::FORWARD,0},ss);
}

void PascalParser::N(Atom where, Atom to, const std::string& ss)
{
  replace_strings.push_back(std::string(1,static_cast<unsigned char>(to&0xFF)));
  pushElements(where,{Entry::REPLACE|Entry::STOP,(replace_strings.size()-1)},ss);
}

void PascalParser::NP(Atom where, const std::string& product, const std::string& ss)
{
  pushElements(where,{Entry::REPLACE|Entry::STOP,store(product)},ss);
}
//RECUERDA QUE EL PRODUCTO NO DEBE CONTENER EL TERMINAL INICIAL; ESE VA EN SS
void PascalParser::TP(Atom where, const std::string& product, const std::string& ss)
{
  pushElements(where,{Entry::REPLACE|Entry::FORWARD,store(product)},ss);
}

unsigned PascalParser::store(const std::string& str)
{
  std::string tmp;
  tmp.reserve(str.size());
  for(auto it = str.rbegin(); it != str.rend(); it++)
    tmp += *it;
  replace_strings.push_back(tmp);
  return replace_strings.size()-1;
}

void PascalParser::terminals()
{
  for(unsigned i = 0; i < static_cast<unsigned>(I_INDEXES::END); i++)
    table[i*width + i] = {Entry::POP|Entry::FORWARD,0};
}
