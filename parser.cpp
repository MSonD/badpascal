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
  TP(0x38,"\x0d\x14\x2b\x14\x3b\x2c\x30\x39\x32");
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
  TP(0x3D,"\x2d\x33\x2e","\x01");
  E(0x3D,"\x30");
  //G
  TP(0x3E,"\x3b\x31\x3c\x30\x3e","\x14");
  E(0x3E,"\x02");
  //S
  N(0x3F,0x40,"\x14");
  N(0x3F,0x41,"\x1d\x1e");
  N(0x3F,0x42,"\x1f\x20");
  N(0x3F,0x46,"\x02");
  N(0x3F,0x48,"\x09\x13\x0e\x08");
  //A
  TP(0x40,"\x36\x4f", "\x14");
  //L
  TP(0x41,"\x2b\x14\x3b\x2c", "\x1d");
  TP(0x41,"\x2b\x14\x3b\x2c", "\x1e");
  //W
  TP(0x42,"\x2b\x43\x2c","\x1f");
  TP(0x42, "\x45","\x20");
  //U
  TP(0x43,"\x45","\x14");
  TP(0x43,"\x44","\x35");
  //Y
  TP(0x44,"\x43","\x2f");
  E(0x44,"\x2c");
  //X
  E(0x45,"\x30\x06\x11\x07");
  TP(0x45,"\x2b\x43\x2c");
  //K
  TP(0x46,"\x02\x3f\x47\x07");
  //P
  TP(0x47,"\x30\x3f\x47");
  E(0x47,"\x11\x07");
  //B
  N(0x48,0x49,"\x09");
  N(0x48,0x4B,"\x0e");
  N(0x48,0x4C,"\x13");
  N(0x48,0x4D,"\x08");
  //I
  TP(0x49,"\x09\x54\x0f\x3f\x4a");
  //O
  T(0x4A,"\x30");
  TP(0x4A,"\x06\x3f");
  //R
  TP(0x4B,"\x0e\x3f\x47\x11\x54");
  //Q
  TP(0x4C,"\x13\x54\x04\x3f");
  //D
  TP(0x4D,"\x08\x40\x4e\x4f\x04\x3f");
  //Z
  T(0x4E,"\x10\x05");//check later
  //E
  NP(0x4F,"\x51\x50", "\x2b\x14\x0b\x33\x34");
  //E'
  TP(0x50,"\x21\x51\x50");
  TP(0x50,"\x22\x51\x50");
  TP(0x50,"\x0C\x51\x50");
  
  E(0x50,"\x25\x26\x27\x28\x29\x2a\x2c\x04\x0f\x10\x05\x30\x06\x11\x07");
  //T
  NP(0x51,"\x53\x52","\x2b\x14\x0b\x33\x34");
  //T'
  TP(0x52,"\x23\x53\x52");
  TP(0x52,"\x24\x53\x52");
  TP(0x52,"\x04\x53\x52");
  TP(0x52,"\x0a\x53\x52");
  TP(0x52,std::string("\x00\x53\x52",3));
  E(0x52,"\x21\x22\x0c\x25\x26\x27\x28\x29\x2a\x2c\x04\x0f\x10\x05\x30\x06\x11\x07");
  //F
  TP(0x53,"\x2b\x4f\x2c");
  T(0x53,"\x33\x34\x14");
  TP(0x53,"\x0b\x14");
  //C
  NP(0x54,"\x4f\x55\x4f","\x2b\x14\x0b\x33\x34");
  //R'
  T(0x55,"\x25\x26\x27\x28\x29\x2a");
}
inline void PascalParser::pushElements(Atom where, Entry e, const std::string& ss)
{
  for(unsigned char i : ss)
    table[where*width+i] = e;
}

inline void PascalParser::pushElements(Atom where, Entry e, Atom ss)
{
    table[where*width+ss] = e;
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

void PascalParser::TP(Atom where, const std::string& product)
{
  printString(product);
  std::cerr<< std::endl;
  pushElements(where,{Entry::REPLACE|Entry::FORWARD,store(product.substr(1,std::string::npos))},product[0]);
}

unsigned PascalParser::store(const std::string& str)
{
  replace_strings.push_back(str);
  return replace_strings.size()-1;
}

void PascalParser::terminals()
{
  for(unsigned i = 0; i < static_cast<unsigned>(I_INDEXES::END); i++)
    table[i*width + i] = {Entry::POP|Entry::FORWARD,0};
}
