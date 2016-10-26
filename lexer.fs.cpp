#include <iostream>
#include "lexer.h"
#include "lexer_base.h"
#include "symtable.h"

///Definir macros usadas por el generador de código fsmm

#define F_FSM_char cur
//Guardar caracter en el buffer
#define F_FSM_STORE_CHAR(cur) {\
  string_out += cur;\
}
//Pedir otro un nuevo carácter
#define F_FSM_REQUEST_CHARACTER(cur) {\
  if(buffer_i == buffer.end()){\
    read(buffer);\
    if(!buffer.size()){\
      buffer.push_back('\0');\
    }\
    buffer_i = buffer.begin();\
    cur = *buffer_i;\
  }else{\
    buffer_i++;\
    cur = *buffer_i;\
  }\
}

//Se ha identificado un token
#define F_FSM_TOKEN_READY_S( typeID, keyID) {\
  write_to.AutoString(typeID,keyID,string_out,tables);\
  string_out =  "";\
}
void TokenFilter::callback(void* ref)
{
  TokenFilter* ptr = reinterpret_cast<TokenFilter*> (ref);
  
}

//Funcion productora de carácteres
unsigned streamTestFun (std::istream& cin,CharBuffer &buffer){
  //asegurar que el buffer sea del tamaño correcto
  buffer.resize(buffer_size);
  //Limpiar banderas del la entrada (no tiene uso en este contexto, por ahora)
  cin.clear();
  //Leer máximo, el tamaño del buffer;
  cin.read(buffer.data(), buffer_size);
  //Reducir extensioón del buffer, al numero de carácteres recibidos
  buffer.resize(cin.gcount());
  //Señalar fin de archivo
  if(buffer.size() == 0) buffer.push_back('\0');
  return cin.gcount();
}
void TokenFilter::setContext(Context& arg){
  context = &arg;
}
void TokenFilter::setBuilder(TokenPusher& arg){
  write_to = &arg;
}

void TokenFilter::setSource(StreamFun arg){
  read = arg;
}

TokenHandle TokenFilter::handle(){
  return TokenHandle{&callback,this,write_to->result(),mux};
}

void TokenFilter::run(){
  lex_stream (read,*write_to, *context);
}
//Función analisis léxico
void lex_stream (StreamFun read,TokenPusher &write_to, Context& tables){
  CharBuffer buffer;
  buffer.reserve(buffer_size);
  CharBuffer::iterator buffer_i = buffer.begin();
  //Carácter actual
  charT cur;
  //buffer de cadena procesada
  std::string string_out;
  
  F_FSM_REQUEST_CHARACTER(cur);
//Estado inicial
F_FSM_default:
  if(isalpha(cur)){
    goto F_FSM_S;
  }
  switch(cur){
    case '(':
      F_FSM_REQUEST_CHARACTER(cur);
      goto F_FSM_parens;
    case ')':
    case '[':
    case ']':
    case ';':
    case '.':
    case ',':
      write_to.Special(Token::SPECIAL,cur);
      break;
    case ':':
      F_FSM_REQUEST_CHARACTER(cur);
      goto F_FSM_colon;
    case '+':
    case '-':
    case '*':
    case '/':
      write_to.Special(Token::A_OPERATOR,cur);
      break;
    case '\n':
      write_to.Endl();
      break;
    case '>':
      F_FSM_REQUEST_CHARACTER(cur);
      goto F_FSM_greater;
    case '<':
      F_FSM_REQUEST_CHARACTER(cur);
      goto F_FSM_lesser;
      break;
    case '=':
      write_to.Special(Token::R_OPERATOR,2);
      break;
    case '0':
      F_FSM_REQUEST_CHARACTER(cur);
      goto F_FSM_zero;
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      F_FSM_STORE_CHAR(cur);
      F_FSM_REQUEST_CHARACTER(cur);
      goto F_FSM_numeric;
    case '\0':
      return;
      break;
    case '\'':
      F_FSM_STORE_CHAR(cur);
      F_FSM_REQUEST_CHARACTER(cur);
      goto F_FSM_string;
    default:
      if(!std::isspace(cur))
      std::cerr << "carácter " << cur << "extraño.\n";
      //TODO: error
  }
  F_FSM_REQUEST_CHARACTER(cur);
  goto F_FSM_default;
//Cadena
F_FSM_string:
  if(cur == '\''){
    F_FSM_STORE_CHAR(cur);
    F_FSM_REQUEST_CHARACTER(cur);
    write_to.String(string_out,tables);
    string_out.clear();
    goto F_FSM_default;
  }
  if(cur == '\0'){
    std::cerr << "literal sin terminar.\n";
    goto F_FSM_default;
  }
  F_FSM_STORE_CHAR(cur);
  F_FSM_REQUEST_CHARACTER(cur);
  goto F_FSM_string;
//Desambiguador : :=
F_FSM_colon:
  if(cur == '='){
    write_to.Special(Token::ASSIGNMENT,0);
    F_FSM_REQUEST_CHARACTER(cur);
    goto F_FSM_default;
  }
  write_to.Special(Token::SPECIAL,':');
  goto F_FSM_default;
//Desambiguar > >=
F_FSM_greater:
  if(cur == '='){
    write_to.Special(Token::R_OPERATOR,5);
    F_FSM_REQUEST_CHARACTER(cur);
    goto F_FSM_default;
  }
  write_to.Special(Token::R_OPERATOR,4);
  goto F_FSM_default;
//< <> <=
F_FSM_lesser:
  switch (cur){
  case '=':
    write_to.Special(Token::R_OPERATOR,1);
    F_FSM_REQUEST_CHARACTER(cur);
    goto F_FSM_default;
  case '>':
    write_to.Special(Token::R_OPERATOR,3);
    F_FSM_REQUEST_CHARACTER(cur);
    goto F_FSM_default;
  }
  write_to.Special(Token::R_OPERATOR,0);
  goto F_FSM_default;
//Paréntesis o comentario
F_FSM_parens:
  if(cur == '*'){
    F_FSM_REQUEST_CHARACTER(cur);
    goto F_FSM_ignore;
  }
  write_to.Special(Token::SPECIAL,'(');
  goto F_FSM_default;
//Comentario
F_FSM_ignore:
  if(cur == '*'){
    F_FSM_REQUEST_CHARACTER(cur);
    goto F_FSM_ignore_expect;
  }
  if(cur == '\n'){
    goto F_FSM_default;
  }
  F_FSM_REQUEST_CHARACTER(cur);
  goto F_FSM_ignore;
//Posible fin de comentario
F_FSM_ignore_expect:
  if(cur == '*'){
    F_FSM_REQUEST_CHARACTER(cur);
    goto F_FSM_ignore_expect;
  }
  if(cur == ')'){
    F_FSM_REQUEST_CHARACTER(cur);
    goto F_FSM_default;
  }
  if(cur == '\n'){
    goto F_FSM_default;
  }
  F_FSM_REQUEST_CHARACTER(cur);
  goto F_FSM_ignore;
//Decimal
F_FSM_numeric:
  if(std::isdigit(cur)){
    F_FSM_STORE_CHAR(cur);
    F_FSM_REQUEST_CHARACTER(cur);
    goto F_FSM_numeric;
  }
  if(cur == '.'){
    F_FSM_STORE_CHAR(cur);
    F_FSM_REQUEST_CHARACTER(cur);
    goto F_FSM_float;
  }
  if(std::isalpha(cur)){
    //TODO:error
    std::cerr << "carácter " << cur << " en literal entero.\n";
    F_FSM_REQUEST_CHARACTER(cur);
    goto F_FSM_numeric;
  }
  write_to.Integer(string_out);
  string_out.clear();
  goto F_FSM_default;
//Flotante
F_FSM_float:
  if(std::isdigit(cur)){
    F_FSM_STORE_CHAR(cur);
    F_FSM_REQUEST_CHARACTER(cur);
    goto F_FSM_float;
  }
  if(std::isalpha(cur)){
    //TODO:error
    std::cerr << "carácter " << cur << " en literal flotante.\n";
    F_FSM_REQUEST_CHARACTER(cur);
    goto F_FSM_float;
  }
  write_to.Float(string_out);
  string_out.clear();
  goto F_FSM_default;
  //AQUI SE GENERA EL CÓDIGO
F_FSM_zero:
  if(cur == 'x'|cur == 'X'){
    F_FSM_REQUEST_CHARACTER(cur)
    goto F_FSM_hex;
  }
  if(cur == '.'){
    F_FSM_STORE_CHAR(cur);
    F_FSM_REQUEST_CHARACTER(cur);
    goto F_FSM_float;
  }
  if(std::isdigit(cur)){
    goto F_FSM_oct;
  }
  
  if(std::isalpha(cur)){
    F_FSM_REQUEST_CHARACTER(cur);
    goto F_FSM_zero;
  }
  write_to.Integer("0");
  F_FSM_REQUEST_CHARACTER(cur);
  goto F_FSM_default;
F_FSM_hex:
  if(std::isxdigit(cur)){
    F_FSM_STORE_CHAR(cur);
    F_FSM_REQUEST_CHARACTER(cur);
    goto F_FSM_hex;
  }
  if(std::isalpha(cur)){
    //TODO: error
    std::cerr << "carácter " << cur << " en literal hexadecimal.\n";
    F_FSM_REQUEST_CHARACTER(cur);
    goto F_FSM_hex;
  }
  write_to.Integer(string_out,16);
  string_out.clear();
  goto F_FSM_default;
F_FSM_oct:
  if(cur == '.'){
    F_FSM_STORE_CHAR(cur);
    F_FSM_REQUEST_CHARACTER(cur);
    goto F_FSM_float;
  }
  if(std::isdigit(cur) && cur < '8'){
    F_FSM_STORE_CHAR(cur);
    F_FSM_REQUEST_CHARACTER(cur);
    goto F_FSM_oct;
  }
  if(std::isalpha(cur) || std::isdigit(cur) ){
    //TODO: error
    std::cerr << "carácter " << cur << " en iteral octal.\n";
    F_FSM_REQUEST_CHARACTER(cur);
    goto F_FSM_oct;
  }
  write_to.Integer(string_out,8);
  string_out.clear();
  goto F_FSM_default;
@
}
//FIN DE CÓDIGO GENERADO
@
AND ARRAY BEGIN DIV DO DOWNTO ELSE END FOR
IF MOD NOT OR PROGRAM REPEAT THEN TO UNTIL
VAR WHILE

boolean$ char$ integer$ real$ text$ 

get! put! read! readln! write! writeln!

abs# chr# cos# eof# eoln# exp# ln# sin# sqr# sqrt# trunc#