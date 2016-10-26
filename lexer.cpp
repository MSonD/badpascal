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
F_FSM_IDSTR:
	if(!F_FSM_IS_IDENTIFIER_CHAR(F_FSM_char) ){
		F_FSM_TOKEN_READY_S(1,0);
		goto F_FSM_default;
	}
	F_FSM_STORE_CHAR(F_FSM_char);
	F_FSM_REQUEST_CHARACTER(F_FSM_char);
	goto F_FSM_IDSTR;
F_FSM_S:
	switch(F_FSM_char){
	case 'A':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SA;
		break;
	case 'B':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SB;
		break;
	case 'D':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SD;
		break;
	case 'E':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SE;
		break;
	case 'F':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SF;
		break;
	case 'I':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SI;
		break;
	case 'M':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SM;
		break;
	case 'N':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SN;
		break;
	case 'O':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SO;
		break;
	case 'P':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SP;
		break;
	case 'R':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SR;
		break;
	case 'T':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_ST;
		break;
	case 'U':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SU;
		break;
	case 'V':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SV;
		break;
	case 'W':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SW;
		break;
	case 'a':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Sa;
		break;
	case 'b':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Sb;
		break;
	case 'c':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Sc;
		break;
	case 'e':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Se;
		break;
	case 'g':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Sg;
		break;
	case 'i':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Si;
		break;
	case 'l':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Sl;
		break;
	case 'p':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Sp;
		break;
	case 'r':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Sr;
		break;
	case 's':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Ss;
		break;
	case 't':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_St;
		break;
	case 'w':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Sw;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SA:
	switch(F_FSM_char){
	case 'N':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SAN;
		break;
	case 'R':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SAR;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SAN:
	switch(F_FSM_char){
	case 'D':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SAND;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SAND:
	if(!F_FSM_IS_IDENTIFIER_CHAR(F_FSM_char) ){
		F_FSM_TOKEN_READY_S(0,0);
		goto F_FSM_default;
	}
			goto F_FSM_IDSTR;
F_FSM_SAR:
	switch(F_FSM_char){
	case 'R':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SARR;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SARR:
	switch(F_FSM_char){
	case 'A':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SARRA;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SARRA:
	switch(F_FSM_char){
	case 'Y':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SARRAY;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SARRAY:
	if(!F_FSM_IS_IDENTIFIER_CHAR(F_FSM_char) ){
		F_FSM_TOKEN_READY_S(0,1);
		goto F_FSM_default;
	}
			goto F_FSM_IDSTR;
F_FSM_SB:
	switch(F_FSM_char){
	case 'E':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SBE;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SBE:
	switch(F_FSM_char){
	case 'G':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SBEG;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SBEG:
	switch(F_FSM_char){
	case 'I':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SBEGI;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SBEGI:
	switch(F_FSM_char){
	case 'N':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SBEGIN;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SBEGIN:
	if(!F_FSM_IS_IDENTIFIER_CHAR(F_FSM_char) ){
		F_FSM_TOKEN_READY_S(0,2);
		goto F_FSM_default;
	}
			goto F_FSM_IDSTR;
F_FSM_SD:
	switch(F_FSM_char){
	case 'I':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SDI;
		break;
	case 'O':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SDO;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SDI:
	switch(F_FSM_char){
	case 'V':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SDIV;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SDIV:
	if(!F_FSM_IS_IDENTIFIER_CHAR(F_FSM_char) ){
		F_FSM_TOKEN_READY_S(0,3);
		goto F_FSM_default;
	}
			goto F_FSM_IDSTR;
F_FSM_SDO:
	if(!F_FSM_IS_IDENTIFIER_CHAR(F_FSM_char) ){
		F_FSM_TOKEN_READY_S(0,4);
		goto F_FSM_default;
	}
		switch(F_FSM_char){
	case 'W':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SDOW;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SDOW:
	switch(F_FSM_char){
	case 'N':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SDOWN;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SDOWN:
	switch(F_FSM_char){
	case 'T':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SDOWNT;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SDOWNT:
	switch(F_FSM_char){
	case 'O':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SDOWNTO;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SDOWNTO:
	if(!F_FSM_IS_IDENTIFIER_CHAR(F_FSM_char) ){
		F_FSM_TOKEN_READY_S(0,5);
		goto F_FSM_default;
	}
			goto F_FSM_IDSTR;
F_FSM_SE:
	switch(F_FSM_char){
	case 'L':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SEL;
		break;
	case 'N':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SEN;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SEL:
	switch(F_FSM_char){
	case 'S':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SELS;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SELS:
	switch(F_FSM_char){
	case 'E':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SELSE;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SELSE:
	if(!F_FSM_IS_IDENTIFIER_CHAR(F_FSM_char) ){
		F_FSM_TOKEN_READY_S(0,6);
		goto F_FSM_default;
	}
			goto F_FSM_IDSTR;
F_FSM_SEN:
	switch(F_FSM_char){
	case 'D':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SEND;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SEND:
	if(!F_FSM_IS_IDENTIFIER_CHAR(F_FSM_char) ){
		F_FSM_TOKEN_READY_S(0,7);
		goto F_FSM_default;
	}
			goto F_FSM_IDSTR;
F_FSM_SF:
	switch(F_FSM_char){
	case 'O':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SFO;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SFO:
	switch(F_FSM_char){
	case 'R':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SFOR;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SFOR:
	if(!F_FSM_IS_IDENTIFIER_CHAR(F_FSM_char) ){
		F_FSM_TOKEN_READY_S(0,8);
		goto F_FSM_default;
	}
			goto F_FSM_IDSTR;
F_FSM_SI:
	switch(F_FSM_char){
	case 'F':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SIF;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SIF:
	if(!F_FSM_IS_IDENTIFIER_CHAR(F_FSM_char) ){
		F_FSM_TOKEN_READY_S(0,9);
		goto F_FSM_default;
	}
			goto F_FSM_IDSTR;
F_FSM_SM:
	switch(F_FSM_char){
	case 'O':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SMO;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SMO:
	switch(F_FSM_char){
	case 'D':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SMOD;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SMOD:
	if(!F_FSM_IS_IDENTIFIER_CHAR(F_FSM_char) ){
		F_FSM_TOKEN_READY_S(0,10);
		goto F_FSM_default;
	}
			goto F_FSM_IDSTR;
F_FSM_SN:
	switch(F_FSM_char){
	case 'O':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SNO;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SNO:
	switch(F_FSM_char){
	case 'T':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SNOT;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SNOT:
	if(!F_FSM_IS_IDENTIFIER_CHAR(F_FSM_char) ){
		F_FSM_TOKEN_READY_S(0,11);
		goto F_FSM_default;
	}
			goto F_FSM_IDSTR;
F_FSM_SO:
	switch(F_FSM_char){
	case 'R':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SOR;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SOR:
	if(!F_FSM_IS_IDENTIFIER_CHAR(F_FSM_char) ){
		F_FSM_TOKEN_READY_S(0,12);
		goto F_FSM_default;
	}
			goto F_FSM_IDSTR;
F_FSM_SP:
	switch(F_FSM_char){
	case 'R':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SPR;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SPR:
	switch(F_FSM_char){
	case 'O':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SPRO;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SPRO:
	switch(F_FSM_char){
	case 'G':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SPROG;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SPROG:
	switch(F_FSM_char){
	case 'R':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SPROGR;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SPROGR:
	switch(F_FSM_char){
	case 'A':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SPROGRA;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SPROGRA:
	switch(F_FSM_char){
	case 'M':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SPROGRAM;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SPROGRAM:
	if(!F_FSM_IS_IDENTIFIER_CHAR(F_FSM_char) ){
		F_FSM_TOKEN_READY_S(0,13);
		goto F_FSM_default;
	}
			goto F_FSM_IDSTR;
F_FSM_SR:
	switch(F_FSM_char){
	case 'E':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SRE;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SRE:
	switch(F_FSM_char){
	case 'P':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SREP;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SREP:
	switch(F_FSM_char){
	case 'E':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SREPE;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SREPE:
	switch(F_FSM_char){
	case 'A':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SREPEA;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SREPEA:
	switch(F_FSM_char){
	case 'T':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SREPEAT;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SREPEAT:
	if(!F_FSM_IS_IDENTIFIER_CHAR(F_FSM_char) ){
		F_FSM_TOKEN_READY_S(0,14);
		goto F_FSM_default;
	}
			goto F_FSM_IDSTR;
F_FSM_ST:
	switch(F_FSM_char){
	case 'H':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_STH;
		break;
	case 'O':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_STO;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_STH:
	switch(F_FSM_char){
	case 'E':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_STHE;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_STHE:
	switch(F_FSM_char){
	case 'N':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_STHEN;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_STHEN:
	if(!F_FSM_IS_IDENTIFIER_CHAR(F_FSM_char) ){
		F_FSM_TOKEN_READY_S(0,15);
		goto F_FSM_default;
	}
			goto F_FSM_IDSTR;
F_FSM_STO:
	if(!F_FSM_IS_IDENTIFIER_CHAR(F_FSM_char) ){
		F_FSM_TOKEN_READY_S(0,16);
		goto F_FSM_default;
	}
			goto F_FSM_IDSTR;
F_FSM_SU:
	switch(F_FSM_char){
	case 'N':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SUN;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SUN:
	switch(F_FSM_char){
	case 'T':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SUNT;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SUNT:
	switch(F_FSM_char){
	case 'I':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SUNTI;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SUNTI:
	switch(F_FSM_char){
	case 'L':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SUNTIL;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SUNTIL:
	if(!F_FSM_IS_IDENTIFIER_CHAR(F_FSM_char) ){
		F_FSM_TOKEN_READY_S(0,17);
		goto F_FSM_default;
	}
			goto F_FSM_IDSTR;
F_FSM_SV:
	switch(F_FSM_char){
	case 'A':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SVA;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SVA:
	switch(F_FSM_char){
	case 'R':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SVAR;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SVAR:
	if(!F_FSM_IS_IDENTIFIER_CHAR(F_FSM_char) ){
		F_FSM_TOKEN_READY_S(0,18);
		goto F_FSM_default;
	}
			goto F_FSM_IDSTR;
F_FSM_SW:
	switch(F_FSM_char){
	case 'H':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SWH;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SWH:
	switch(F_FSM_char){
	case 'I':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SWHI;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SWHI:
	switch(F_FSM_char){
	case 'L':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SWHIL;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SWHIL:
	switch(F_FSM_char){
	case 'E':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_SWHILE;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_SWHILE:
	if(!F_FSM_IS_IDENTIFIER_CHAR(F_FSM_char) ){
		F_FSM_TOKEN_READY_S(0,19);
		goto F_FSM_default;
	}
			goto F_FSM_IDSTR;
F_FSM_Sa:
	switch(F_FSM_char){
	case 'b':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Sab;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Sab:
	switch(F_FSM_char){
	case 's':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Sabs;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Sabs:
	if(!F_FSM_IS_IDENTIFIER_CHAR(F_FSM_char) ){
		F_FSM_TOKEN_READY_S(3,0);
		goto F_FSM_default;
	}
			goto F_FSM_IDSTR;
F_FSM_Sb:
	switch(F_FSM_char){
	case 'o':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Sbo;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Sbo:
	switch(F_FSM_char){
	case 'o':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Sboo;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Sboo:
	switch(F_FSM_char){
	case 'l':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Sbool;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Sbool:
	switch(F_FSM_char){
	case 'e':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Sboole;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Sboole:
	switch(F_FSM_char){
	case 'a':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Sboolea;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Sboolea:
	switch(F_FSM_char){
	case 'n':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Sboolean;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Sboolean:
	if(!F_FSM_IS_IDENTIFIER_CHAR(F_FSM_char) ){
		F_FSM_TOKEN_READY_S(2,0);
		goto F_FSM_default;
	}
			goto F_FSM_IDSTR;
F_FSM_Sc:
	switch(F_FSM_char){
	case 'h':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Sch;
		break;
	case 'o':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Sco;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Sch:
	switch(F_FSM_char){
	case 'a':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Scha;
		break;
	case 'r':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Schr;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Scha:
	switch(F_FSM_char){
	case 'r':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Schar;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Schar:
	if(!F_FSM_IS_IDENTIFIER_CHAR(F_FSM_char) ){
		F_FSM_TOKEN_READY_S(2,1);
		goto F_FSM_default;
	}
			goto F_FSM_IDSTR;
F_FSM_Schr:
	if(!F_FSM_IS_IDENTIFIER_CHAR(F_FSM_char) ){
		F_FSM_TOKEN_READY_S(3,1);
		goto F_FSM_default;
	}
			goto F_FSM_IDSTR;
F_FSM_Sco:
	switch(F_FSM_char){
	case 's':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Scos;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Scos:
	if(!F_FSM_IS_IDENTIFIER_CHAR(F_FSM_char) ){
		F_FSM_TOKEN_READY_S(3,2);
		goto F_FSM_default;
	}
			goto F_FSM_IDSTR;
F_FSM_Se:
	switch(F_FSM_char){
	case 'o':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Seo;
		break;
	case 'x':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Sex;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Seo:
	switch(F_FSM_char){
	case 'f':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Seof;
		break;
	case 'l':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Seol;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Seof:
	if(!F_FSM_IS_IDENTIFIER_CHAR(F_FSM_char) ){
		F_FSM_TOKEN_READY_S(3,3);
		goto F_FSM_default;
	}
			goto F_FSM_IDSTR;
F_FSM_Seol:
	switch(F_FSM_char){
	case 'n':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Seoln;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Seoln:
	if(!F_FSM_IS_IDENTIFIER_CHAR(F_FSM_char) ){
		F_FSM_TOKEN_READY_S(3,4);
		goto F_FSM_default;
	}
			goto F_FSM_IDSTR;
F_FSM_Sex:
	switch(F_FSM_char){
	case 'p':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Sexp;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Sexp:
	if(!F_FSM_IS_IDENTIFIER_CHAR(F_FSM_char) ){
		F_FSM_TOKEN_READY_S(3,5);
		goto F_FSM_default;
	}
			goto F_FSM_IDSTR;
F_FSM_Sg:
	switch(F_FSM_char){
	case 'e':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Sge;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Sge:
	switch(F_FSM_char){
	case 't':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Sget;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Sget:
	if(!F_FSM_IS_IDENTIFIER_CHAR(F_FSM_char) ){
		F_FSM_TOKEN_READY_S(4,0);
		goto F_FSM_default;
	}
			goto F_FSM_IDSTR;
F_FSM_Si:
	switch(F_FSM_char){
	case 'n':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Sin;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Sin:
	switch(F_FSM_char){
	case 't':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Sint;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Sint:
	switch(F_FSM_char){
	case 'e':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Sinte;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Sinte:
	switch(F_FSM_char){
	case 'g':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Sinteg;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Sinteg:
	switch(F_FSM_char){
	case 'e':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Sintege;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Sintege:
	switch(F_FSM_char){
	case 'r':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Sinteger;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Sinteger:
	if(!F_FSM_IS_IDENTIFIER_CHAR(F_FSM_char) ){
		F_FSM_TOKEN_READY_S(2,2);
		goto F_FSM_default;
	}
			goto F_FSM_IDSTR;
F_FSM_Sl:
	switch(F_FSM_char){
	case 'n':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Sln;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Sln:
	if(!F_FSM_IS_IDENTIFIER_CHAR(F_FSM_char) ){
		F_FSM_TOKEN_READY_S(3,6);
		goto F_FSM_default;
	}
			goto F_FSM_IDSTR;
F_FSM_Sp:
	switch(F_FSM_char){
	case 'u':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Spu;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Spu:
	switch(F_FSM_char){
	case 't':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Sput;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Sput:
	if(!F_FSM_IS_IDENTIFIER_CHAR(F_FSM_char) ){
		F_FSM_TOKEN_READY_S(4,1);
		goto F_FSM_default;
	}
			goto F_FSM_IDSTR;
F_FSM_Sr:
	switch(F_FSM_char){
	case 'e':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Sre;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Sre:
	switch(F_FSM_char){
	case 'a':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Srea;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Srea:
	switch(F_FSM_char){
	case 'd':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Sread;
		break;
	case 'l':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Sreal;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Sread:
	if(!F_FSM_IS_IDENTIFIER_CHAR(F_FSM_char) ){
		F_FSM_TOKEN_READY_S(4,2);
		goto F_FSM_default;
	}
		switch(F_FSM_char){
	case 'l':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Sreadl;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Sreadl:
	switch(F_FSM_char){
	case 'n':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Sreadln;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Sreadln:
	if(!F_FSM_IS_IDENTIFIER_CHAR(F_FSM_char) ){
		F_FSM_TOKEN_READY_S(4,3);
		goto F_FSM_default;
	}
			goto F_FSM_IDSTR;
F_FSM_Sreal:
	if(!F_FSM_IS_IDENTIFIER_CHAR(F_FSM_char) ){
		F_FSM_TOKEN_READY_S(2,3);
		goto F_FSM_default;
	}
			goto F_FSM_IDSTR;
F_FSM_Ss:
	switch(F_FSM_char){
	case 'i':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Ssi;
		break;
	case 'q':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Ssq;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Ssi:
	switch(F_FSM_char){
	case 'n':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Ssin;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Ssin:
	if(!F_FSM_IS_IDENTIFIER_CHAR(F_FSM_char) ){
		F_FSM_TOKEN_READY_S(3,7);
		goto F_FSM_default;
	}
			goto F_FSM_IDSTR;
F_FSM_Ssq:
	switch(F_FSM_char){
	case 'r':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Ssqr;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Ssqr:
	if(!F_FSM_IS_IDENTIFIER_CHAR(F_FSM_char) ){
		F_FSM_TOKEN_READY_S(3,8);
		goto F_FSM_default;
	}
		switch(F_FSM_char){
	case 't':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Ssqrt;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Ssqrt:
	if(!F_FSM_IS_IDENTIFIER_CHAR(F_FSM_char) ){
		F_FSM_TOKEN_READY_S(3,9);
		goto F_FSM_default;
	}
			goto F_FSM_IDSTR;
F_FSM_St:
	switch(F_FSM_char){
	case 'e':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Ste;
		break;
	case 'r':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Str;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Ste:
	switch(F_FSM_char){
	case 'x':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Stex;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Stex:
	switch(F_FSM_char){
	case 't':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Stext;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Stext:
	if(!F_FSM_IS_IDENTIFIER_CHAR(F_FSM_char) ){
		F_FSM_TOKEN_READY_S(2,4);
		goto F_FSM_default;
	}
			goto F_FSM_IDSTR;
F_FSM_Str:
	switch(F_FSM_char){
	case 'u':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Stru;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Stru:
	switch(F_FSM_char){
	case 'n':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Strun;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Strun:
	switch(F_FSM_char){
	case 'c':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Strunc;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Strunc:
	if(!F_FSM_IS_IDENTIFIER_CHAR(F_FSM_char) ){
		F_FSM_TOKEN_READY_S(3,10);
		goto F_FSM_default;
	}
			goto F_FSM_IDSTR;
F_FSM_Sw:
	switch(F_FSM_char){
	case 'r':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Swr;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Swr:
	switch(F_FSM_char){
	case 'i':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Swri;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Swri:
	switch(F_FSM_char){
	case 't':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Swrit;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Swrit:
	switch(F_FSM_char){
	case 'e':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Swrite;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Swrite:
	if(!F_FSM_IS_IDENTIFIER_CHAR(F_FSM_char) ){
		F_FSM_TOKEN_READY_S(4,4);
		goto F_FSM_default;
	}
		switch(F_FSM_char){
	case 'l':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Swritel;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Swritel:
	switch(F_FSM_char){
	case 'n':
		F_FSM_STORE_CHAR(F_FSM_char);
		F_FSM_REQUEST_CHARACTER(F_FSM_char);
		goto F_FSM_Swriteln;
		break;
	default:
		goto F_FSM_IDSTR;
	}
F_FSM_Swriteln:
	if(!F_FSM_IS_IDENTIFIER_CHAR(F_FSM_char) ){
		F_FSM_TOKEN_READY_S(4,5);
		goto F_FSM_default;
	}
			goto F_FSM_IDSTR;

}
//FIN DE CÓDIGO GENERADO
