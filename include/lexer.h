#ifndef PAS_LEXER_H
#define PAS_LEXER_H
#include <vector>
#include <iostream>
#include <functional>
#include <mutex>
#include "symtable.h"
#include "lexer_base.h"
//Buffer de carácteres
//Funcion de prueba de recepcion de caracteres
unsigned streamTestFun (std::istream& ,CharBuffer &buffer);
//Funcion que hace el analisis lexico propiamente
void lex_stream (StreamFun read,  TokenPusher& write_to, Context& tables);

class TokenFilter{
  Context* context;
  TokenPusher* write_to;
  StreamFun read;
public:
  void setContext(Context&);
  void setBuilder(TokenPusher&);
  void setSource(StreamFun);
  void operator()();
};

#endif