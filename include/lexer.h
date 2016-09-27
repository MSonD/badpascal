#ifndef PAS_LEXER_H
#define PAS_LEXER_H
#include <vector>
#include <iostream>
#include <functional>
#include "symtable.h"
#include "lexer_base.h"
//Buffer de carácteres
using CharBuffer = Vector<charT>;
//Funcion de recepción de carácteres
using StreamFun = std::function<unsigned (CharBuffer&)>;
//Funcion de prueba de recepcion de caracteres
unsigned streamTestFun (std::istream& ,CharBuffer &buffer);
//Funcion que hace el analisis lexico propiamente
void lex_stream (StreamFun read,  TokenPusher& write_to, Context& tables);
#endif