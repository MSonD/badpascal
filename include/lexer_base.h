#ifndef PAS_LEXER_BASE_HPP
#define PAS_LEXER_BASE_HPP
#include <memory>
#include <vector>
#include <cctype>
#define F_FSM_IS_IDENTIFIER_CHAR(c) (std::isalnum(c))
constexpr unsigned buffer_size = 512;
using charT = char;

//Nombres de clases
const std::string S_class_names[] = {
  "KEYWORD",
  "ID",
  "TYPE",
  "FUNCTION",
  "PROCEDURE",
  "A_OPERATOR",
  "R_OPERATOR",
  "SPECIAL",
  "INT_LITERAL",
  "FLT_LITERAL",
  "STRING",
  "ASSIGNMENT"
};
//Nombre de palabras clave
const std::string  S_keywords []= {
"AND","ARRAY","BEGIN","DIV","DO","DOWNTO","ELSE","END","FOR"
,"IF","MOD","NOT","OR","PROGRAM","REPEAT","THEN","TO","UNTIL"
,"VAR","WHILE"
};
//Nombres de tipos
const std::string S_types [] = {
"boolean" ,"char", "integer", "real", "text"
};
//Nombres de procedimientos
const std::string  S_procedures [] = {
"get","put","read","readln","write","writeln"
};
//Nombres de funciones
const std::string  S_functions [] = {
"abs","chr","cos","eof","eoln","exp","ln","sin","sqr","sqrt","trunc"
}; 
//Nombres de operadores
const std::string  S_relational [] = {
  "<","<=","=","<>",">",">="
}; 
class Token{
  public:
  //Tipos o clases de tokens
  enum types : unsigned{
    KEYWORD,
    ID,
    TYPE,
    FUNCTION,
    PROCEDURE,
    A_OPERATOR,
    R_OPERATOR,
    SPECIAL,
    INT_LITERAL,
    FLT_LITERAL,
    STRING,
    ASSIGNMENT
  };
  unsigned type;
  unsigned pos;
  union{
    long id;
    double idf;
  };
  Token();
  Token(unsigned type,long id, unsigned pos = 0);
  Token(unsigned type,double id, unsigned pos = 0);
  unsigned getID();
  friend std::ostream& operator<< (std::ostream &os,const Token& tok);
};
 std::ostream& operator<< (std::ostream &os,const Token& tok);
#endif