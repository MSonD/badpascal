#ifndef PAS_LEXER_BASE_HPP
#define PAS_LEXER_BASE_HPP
#include <memory>
#include <vector>
#include <cctype>
#include <functional>
#include <mutex>
#include "data.h"
#define F_FSM_IS_IDENTIFIER_CHAR(c) (std::isalnum(c))

template <typename T, size_t N>
char ( &_ArraySizeHelper( T (&array)[N] ))[N];

#define sizeof_a( array ) (sizeof( _ArraySizeHelper( array ) ))

constexpr unsigned buffer_size = 512;
using charT = char;
using Atom = unsigned;

using CharBuffer = Vector<charT>;
//Funcion de recepción de carácteres
using StreamFun = std::function<unsigned (CharBuffer&)>;

class Token;


//Nombres de clases
const std::string S_class_names[] = {
  "KEYWORD",
  "ID",
  "TYPE",
  "FUNCT",
  "PROCED",
  "A_OPER",
  "R_OPER",
  "SPECIAL",
  "INT_LIT",
  "FLT_LIT",
  "STRING",
  "ASSIGN",
  "EOF"
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

const std::string  S_special [] = {
  "(",")","[","]",",",";",":","."
};

const std::string  S_arith [] = {
  "+","-","*","/"
};
//Nombres de operadores
const std::string  S_relational [] = {
  "<","<=","=","<>",">",">="
};

const std::string S_atoms [] = {
  "&", "y", "e", "\\", "z", "d", "o", "f", "p", "s", "%", "!", "|", "#", "@", "h", "j", "k", "v", "q",
  "a",
  "b","c","i","r", "↓",
  "¢",
  "g", "u","l", "t", "w", "m",
  "+","-","*","/",
  "<","_","¿","?",">","$",
  "(",")","[","]",",",";",":",".",
  "n",
  "x",
  "\"",
  "=",
  "⊢",
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

enum class I_INDEXES :  unsigned{
  KEYWORD = 0,
  ID = sizeof_a(S_keywords),
  TYPE,
  FUNCTION = TYPE + sizeof_a(S_types),
  PROCEDURE,
  A_OPERATOR = PROCEDURE + sizeof_a(S_procedures),
  R_OPERATOR = A_OPERATOR + sizeof_a(S_arith),
  SPECIAL = R_OPERATOR + sizeof_a(S_relational),
  INT_LITERAL = SPECIAL + sizeof_a(S_special),
  FLT_LITERAL,
  STRING,
  ASSIGNMENT,
  FEOF,
  END
};


class Token{
  public:
  //Tipos o clases de tokens
  enum types : unsigned{
    KEYWORD = 0,
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
    ASSIGNMENT,
    FEOF
  };
  unsigned type;
  unsigned pos;
  union{
    long id;
    double idf;
  };
  Atom atom();
  Token();
  Token(unsigned type,long id, unsigned pos = 0);
  Token(unsigned type,double id, unsigned pos = 0);
  unsigned getID();
  friend std::ostream& operator<< (std::ostream &os,const Token& tok);
};
 std::ostream& operator<< (std::ostream &os,const Token& tok);

 using TokenBuffer = Vector<Token>;
 
 struct TokenHandle{
   TokenBuffer* data;
   std::mutex* mux;
   std::mutex* rev_mux;
 };

 
#endif