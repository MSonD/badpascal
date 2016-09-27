#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <algorithm>
#include <atomic>
#include "lexer.h"
#include "symtable.h"

//clase que procesa los tokens generados por el FSM.
//no es la versión final.
class TokenTester : public TokenPusher{
  //Lista de tokens que se han reconocido
  Vector<Token> list;
  //linea actual
  unsigned line;
  //Vector2<int> test;
public:
  void String(const std::string& string, Context &tables){
    auto it = tables.strs.find(string);
    if(it == tables.strs.end()){
      tables.strs[string] = tables.strs_i;
      list.push_back(Token(Token::STRING,(long) tables.strs_i++,line));
    }else{
      list.push_back(Token(Token::STRING,(long) *(it.second),line));
    }
  }
  void ID(const std::string& string, Context &tables){
    auto it = tables.ids.find(string);
    if(it == tables.ids.end()){
      tables.ids[string] = tables.ids_i;
      list.push_back(Token(Token::ID,(long) tables.ids_i++,line));
    }else{
      list.push_back(Token(Token::ID,(long) *(it.second),line));
    }
  }
  
  void Language(unsigned type,long id){
    list.push_back(Token(type,id,line));
  }
  
  void Integer(const std::string &str, int base = 10){
    list.push_back(Token(Token::INT_LITERAL,std::stol(str,0,base),line) );
  }
  void Float(const std::string &str){
    list.push_back(Token(Token::FLT_LITERAL,std::stod(str),line) );
  }
  void Special(unsigned type, unsigned subtype){
    list.push_back(Token(type,(long) subtype,line));
  }
  void Endl(){
    line++;
  }
  void printTokens(std::ostream& cout){
    cout << "#\tValor\tID\tClase\n";
    for(unsigned i = 0;i < list.size();i++){
      cout << i << '\t' << list[i] << '\t' << list[i].getID() <<'\t'<<
	list[i].type << ':' << S_class_names[ list[i].type]<< '\n' ;
    }
    cout << std::endl;
  }
  
};

void printStrings(std::ostream& cout, Context& context) {
    cout << "#\tValor\n";
    for(auto elem : context.strs){
      cout << *elem.second << '\t' << *elem.first << '\n';
    }
    cout << std::endl;
}

void printIDs(std::ostream& cout,  Context& context) {
    cout << "#\tValor\n";
    for(auto elem : context.ids){
      cout << *elem.second << '\t' << *elem.first << '\n';
    }
    cout << std::endl;
}
#if  0
int lmain(){
  Vector<Token> tok;
  tok.reserve(50);
  tok.push_back(Token(2,1L,2));
  tok.push_back(Token(2,1L,2));
  tok.resize(50);
  long i = 0;
  for(auto &it : tok){
    it = Token(1,i,1);
    i++;
  }
  tok.push_back(Token(2,1L,2));
  tok.resize(20);
  for(auto &it : tok){
    std::cout << it << std::endl;
  }
  //auto x = fun["GoE"];
}
//Q210
int main(){
  Hash2<std::string,unsigned> fun;
  fun["GEE"] = 70;
  fun["GOO"] = 71;
  fun["GLEE"] = 72;
  fun["GOETHE"] = 73;
  fun["GOETHE"] = 173;
  short ai;
  for(auto it:fun){
      std::cout<< *it.second << " ";
  }
  int i;
}
#endif
int main(int argc, char **argv) {
  
  //Seccion de manejo de archivo
  std::ostream* output;
  std::istream* input;
  std::ifstream file_in; 
  std::ofstream file_out;
  switch(argc){
    case 1:
      input = &std::cin;
      output = &std::cout;
      break;
    case 2:
      file_in.open(argv[1],std::ios::in);
      if(!file_in){
	std::cerr << "No se puede abrir " << argv[1] << " para lectura"  << std::endl;
	return -1;
      }
      input = &file_in;
      output = &std::cout;
      break;
    case 3:
      file_in.open(argv[1]);
      if(file_in.fail()){
	std::cerr << "No se puede abrir " << argv[1] << " para lectura" << std::endl;
	return -1;
      }
      file_out.open(argv[2]);
      if(file_out.fail()){
	std::cerr << "No se puede abrir " << argv[2] << " para escritura" << std::endl;
	return -1;
      }
      input = &file_in;
      output = &file_out;
      break;
    default:
      std::cerr << "Uso: badpas [archivo_entrada] [archivo_salida]" << std::endl;
      return -1;
  }
  //Buffer de caracteres leidos
  CharBuffer p;
  //Tablas auxiliares
  Context context;
  //Consumidor de tokens
  TokenTester default_out;
  //Reservar espacio para cadena
  p.reserve(buffer_size);
  //Realizar análisis lexico
  lex_stream(
    [input](CharBuffer &buffer){return streamTestFun(*input,buffer);},default_out,context
  );
  //Imprimir lista de tokens
  default_out.printTokens(*output);
  //Imprimir las otras tablas
  *output << "CADENAS\n";
  printStrings(*output,context);
  *output << "IDENTIFICADORES\n";
  printIDs(*output,context);
}
