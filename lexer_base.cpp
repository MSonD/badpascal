#include "lexer.h"
#include <iostream>
//Constructor con argumento entero
Token::Token():type(-1),id(0),pos(-1){}
Token::Token(unsigned int type_, long id_, unsigned int pos_):
type(type_), id(id_), pos(pos_)
{}
//Constructor 
Token::Token(unsigned int type_, double id_, unsigned int pos_):
type(type_), idf(id_), pos(pos_)
{}

std::ostream& operator<< (std::ostream &os,const Token& tok){
  switch(tok.type){
    case Token::KEYWORD:
      os << S_keywords[tok.id];
      break;
    case Token::ID:
      os << std::string("id[") << tok.id <<"]";
      break;
    case Token::TYPE:
      os << S_types[tok.id];
      break;
    case Token::FUNCTION:
      os << S_functions[tok.id];
      break;
    case Token::PROCEDURE :
      os << S_procedures[tok.id];
      break;
    case Token::SPECIAL:
      os << static_cast<char>(tok.id);
      break;
    case Token::R_OPERATOR:
      os << S_relational[tok.id];
      break;
    case Token::A_OPERATOR:
      os << static_cast<char>(tok.id);
      break;
    case Token::INT_LITERAL:
      os << tok.id;
      break;
    case Token::FLT_LITERAL:
      os << tok.idf;
      break;
    case Token::STRING:
      os << "str[" << tok.id<<"]";
      break;
    case Token::ASSIGNMENT:
      os << ":=";
      break;
  }
  return os;
}
//Evita que salga basura cuando se escribe un flotante
unsigned int Token::getID()
{
  if(type != Token::INT_LITERAL && type !=  Token::FLT_LITERAL)
    return id;
  return 0;
}
