#include "lexer.h"
#include <iostream>
#include <assert.h>
const unsigned AtomTypeIndex [] = {
  static_cast<unsigned>(I_INDEXES::KEYWORD),
  static_cast<unsigned>(I_INDEXES::ID),
  static_cast<unsigned>(I_INDEXES::TYPE),
  static_cast<unsigned>(I_INDEXES::FUNCTION),
  static_cast<unsigned>(I_INDEXES::PROCEDURE),
  static_cast<unsigned>(I_INDEXES::A_OPERATOR),
  static_cast<unsigned>(I_INDEXES::R_OPERATOR),
  static_cast<unsigned>(I_INDEXES::SPECIAL),
  static_cast<unsigned>(I_INDEXES::INT_LITERAL),
  static_cast<unsigned>(I_INDEXES::FLT_LITERAL),
  static_cast<unsigned>(I_INDEXES::STRING),
  static_cast<unsigned>(I_INDEXES::ASSIGNMENT),
  static_cast<unsigned>(I_INDEXES::END)
};

//Constructor con argumento entero
Token::Token():type(-1),id(0),pos(-1){}
Token::Token(unsigned int type_, long id_, unsigned int pos_):
type(type_), id(id_), pos(pos_)
{}
//Constructor 
Token::Token(unsigned int type_, double id_, unsigned int pos_):
type(type_), idf(id_), pos(pos_)
{}

Atom Token::atom()
{
  switch (type){
    case TYPE:
    case PROCEDURE:
    case R_OPERATOR:
    case KEYWORD:
      return AtomTypeIndex[type]+id;
    case INT_LITERAL:
    case FLT_LITERAL:
    case ID:
    case STRING:
    case ASSIGNMENT:
      return AtomTypeIndex[type];
    case SPECIAL:
      unsigned off;
      switch(id){
	case '(':
	  off  = 0;
	  break;
	case ')':
	  off =  1;
	  break;
	case '[':
	  off =  2;
	  break;
	case ']':
	  off = 3;
	  break;
	case ',':
	  off = 4;
	  break;
	case ';':
	  off = 5;
	  break;
	case ':':
	  off = 6;
	  break;
	case '.':
	  off = 7;
	  break;
	default:
	  off = 8;
	  assert(0);
	  //THIS DOES NOT HAPPEN
      }
      return AtomTypeIndex[type]+off;
      break;
    case A_OPERATOR:
      switch(id){
	case '+':
	  off  = 0;
	  break;
	case '-':
	  off =  1;
	  break;
	case '*':
	  off =  2;
	  break;
	case '/':
	  off = 3;
	  break;
	default:
	  off = 4;
	  assert(0);
	  //THIS SHOULD NOT EVER HAPPEN
      }
      return AtomTypeIndex[type]+off;
    default:
	//THIS IS BAD
	assert(0);
  }
}

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
