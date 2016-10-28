
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <algorithm>
#include <atomic>
#include <mutex>
#include <thread>
#include <utility>
#include "lexer.h"
#include "symtable.h"
#include "parser.h"

//clase que procesa los tokens generados por el FSM.
//no es la versión final.
class TokenTester : public TokenPusher{
  using Lock = std::lock_guard<std::mutex>;
  std::mutex buffer_mux;
  std::mutex termination_mux;
  //Lista de tokens que se han reconocido
  Vector<Token> list;
  //linea actual
  unsigned line = 1;
  //Vector2<int> test;
public:
  void String(const std::string& string, Context &tables){
    auto it = tables.strs.find(string);
    if(it == tables.strs.end()){
      tables.strs[string] = tables.strs_i;
      Lock lock(buffer_mux);
      list.push_back(Token(Token::STRING,(long) tables.strs_i++,line));
    }else{
      Lock lock(buffer_mux);
      list.push_back(Token(Token::STRING,(long) *(it.second),line));
    }
  }
  void ID(const std::string& string, Context &tables){
    auto it = tables.ids.find(string);
    if(it == tables.ids.end()){
      tables.ids[string] = tables.ids_i;
      Lock lock(buffer_mux);
      list.push_back(Token(Token::ID,(long) tables.ids_i++,line));
    }else{
      Lock lock(buffer_mux);
      list.push_back(Token(Token::ID,(long) *(it.second),line));
    }
  }
  
  void Language(unsigned type,long id){
    Lock lock(buffer_mux);
    list.push_back(Token(type,id,line));
  }
  
  void Integer(const std::string &str, int base = 10){
    Lock lock(buffer_mux);
    list.push_back(Token(Token::INT_LITERAL,std::stol(str,0,base),line) );
  }
  void Float(const std::string &str){
    Lock lock(buffer_mux);
    list.push_back(Token(Token::FLT_LITERAL,std::stod(str),line) );
  }
  void Special(unsigned type, unsigned subtype){
    Lock lock(buffer_mux);
    list.push_back(Token(type,(long) subtype,line));
  }
  void Endl(){
    line++;
  }
  void Eof(){
    buffer_mux.lock();
    list.push_back(Token(Token::FEOF,0L,line));
    buffer_mux.unlock();
    //TODO: verificar dependencia
    std::lock_guard<std::mutex> lockd (termination_mux);
  }
  
  TokenHandle handle(){
    return TokenHandle{&list,&buffer_mux,&termination_mux};
  }
  TokenBuffer& result(){
    return list;
  }
  TokenTester(){
    //4 HORAS 4 HORAS!!!!!!
    buffer_mux.lock();
    termination_mux.lock();
    buffer_mux.unlock();
    termination_mux.unlock();
  }
};
//Interface entre el buffer de tokens y el parser, que lee de uno por uno
class TokenAtomAdapter : public AtomSource{
  using Lock = std::lock_guard<std::mutex>;
  std::ostream* out = nullptr;
  TokenHandle handle;
  TokenBuffer local_buffer;
  bool finished_work = false;
  Token last;
  size_t rbuffer_index = 0;
  size_t nline = 1;
public:
  //Cambia el bufer con que se sincroniza
  void setHandle(TokenHandle hl){
    handle = hl;
    hl.rev_mux->lock();
  }
  //Linea actual
  unsigned line() {
    return nline;
  }
  void setOutput(std::ostream& stream){
    out = &stream;
  }
  //Trae un token
  Atom fetch(){
    if(finished_work){
      return last.atom();
    }
    while(local_buffer.size() == 0){
      {//TODO: eliminar busy-wait mientras se llena el buffer (si eso eso realmente llega a pasar)
	Lock lock(*handle.mux);
	swap(*(handle.data),local_buffer);
      }
      rbuffer_index = 0;
      //TODO: REMOVE IN FINAL VERSION
      //La tabla de tokens es efímera
      printTokens();
    }
    
    auto token = local_buffer[rbuffer_index++];

    nline = token.pos;
    last = token;

    if(token.type == Token::FEOF){
      handle.rev_mux->unlock();
      finished_work = true;
    }
    if(rbuffer_index == local_buffer.size())
      local_buffer.resize(0,false);
    return token.atom();
  }
  //IMprime tabla
  void printTokens(){
    std::ostream& cout = *out;
    cout << "#\tValor\tID\tClase\tIDAtomo\tAtomo\n";
    for(unsigned i = 0;i < local_buffer.size();i++){
      cout << i << '\t' << local_buffer[i] << '\t' << local_buffer[i].getID() <<'\t'<<
	local_buffer[i].type << ':' << S_class_names[ local_buffer[i].type]<< '\t'<< local_buffer[i].atom()<< '\t' << S_atoms[local_buffer[i].atom()] << '\t' <<local_buffer[i].pos <<'\n'  ;
    }
    cout << std::endl;
  }
};

//Imprime cadenas guardadas
void printStrings(std::ostream& cout, Context& context) {
    cout << "#\tValor\n";
    for(auto elem : context.strs){
      cout << *elem.second << '\t' << *elem.first << '\n';
    }
    cout << std::endl;
}
//Imprime identificadores guardados
void printIDs(std::ostream& cout,  Context& context) {
    cout << "#\tValor\n";
    for(auto elem : context.ids){
      cout << *elem.second << '\t' << *elem.first << '\n';
    }
    cout << std::endl;
}

#ifdef TEST_MODE
int main0 (int argc, char **argv, Context& context, TokenTester& default_out)
#else
int main (int argc, char **argv)
#endif
 {
  
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
  //Tablas auxiliares
#ifndef TEST_MODE
  Context context;
  //Consumidor de tokens
  TokenTester default_out;
#endif
  //Lexer
  TokenFilter Tmachine;
  //Adaptador
  TokenAtomAdapter pipe;
  pipe.setOutput(*output);
  //Parser
  PascalParser Pmachine;
  //Conectar adaptador con fuente de tokens
  pipe.setHandle(default_out.handle());
  //Conectar parser conn adaptador
  Pmachine.setSource(pipe);
  //Configurar fuente de tokens
  Tmachine.setBuilder(default_out);
  Tmachine.setContext(context);
  //Conectar fuente de tokens con fuente de cadenas
  Tmachine.setSource([input](CharBuffer &buffer){return streamTestFun(*input,buffer);});
  //Iniciar threads
  std::thread lexer(std::move(Tmachine));
  std::thread parser(std::move(Pmachine));
  parser.join();
  lexer.join();
  
  //Imprimir lista de tokens
  //default_out.printTokens(*output);
  //Imprimir las otras tablas
  *output << "CADENAS\n";
  printStrings(*output,context);
  *output << "IDENTIFICADORES\n";
  printIDs(*output,context);
    
}
