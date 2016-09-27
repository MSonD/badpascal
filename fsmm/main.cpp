/****
 * FSMm => un muy pequeño compilador de listas de palabras a FSM C/C++
 ****/
#include <iostream>
#include <fstream>
#include <algorithm>
#include <list>
#include <string>
#include <limits>
using std::string;

const string prefix = "F_FSM_";
//Nombre de variable
const string char_name = prefix + "char";
//Etiqueta a implementar
const string default_case = "F_FSM_default";
//Macros/funciones a implementar
const string request_char = prefix + "REQUEST_CHARACTER";
const string token_ready =  prefix + "TOKEN_READY_S";
const string save_char =  prefix + "STORE_CHAR";
const string is_identifier =  prefix + "IS_IDENTIFIER_CHAR";
const string append =  prefix + "APPEND";
const string id_label= prefix + "IDSTR";

//Tipo de cadena identificada
enum class StrType{
  none,
  keyword,
  proc_builtin,
  fun_builtin,
  type_builtin,
  id
};
//Obtiene el numero de clase de token
int classNb(StrType x){
  switch(x){
    case StrType::none: return -1; break;
    case StrType::id: return 1; break;
    case StrType::keyword: return 0; break;
    case StrType::proc_builtin: return 4; break;
    case StrType::fun_builtin: return 3; break;
    case StrType::type_builtin: return 2; break;
    default: return -1;
  }
}
/***
 * Clase que representa un nodo o arbol trie
 * Usa una para represntar cada nodo
 * **/
template<class C>
class TableEntry{
  using List = std::list<TableEntry<C> >;
public:
  C elem;
  StrType strtype;
  int level;
  std::list<TableEntry> ptr;
  
  TableEntry(): strtype(StrType::none) {}
  TableEntry(C e, StrType term, int lvl = 0): elem(e), strtype(term), level(lvl){};
  
  //NOTE: typename? Para que el compilador no se confunda por alguna razón
  
  ///Insertar y construir un nodo trie en este nodo
  typename List::iterator insert(C e, StrType b, int lvl = 0){
    auto first = ptr.begin();
    auto last= ptr.end();
    while (first!=last) {
      if(e < first->elem) break;
      if(e == first->elem){
	return first;
      }
      ++first;
    }
    return ptr.insert(first, TableEntry(e,b,lvl) );
  }
  
  ///Crear un sub-arbol correspondiente a la cadena, maneja coincidencias
  ///El carácter final de la cadena se establece a terminal
  void pushString(const string &s,StrType type = StrType::none,int idx = 0){
    TableEntry<C>* straw = this;
    for(long i = 0; i < static_cast<long>(s.size())-1; i++){
      //insertar en elemento actual, 'dereferenciar' iterador y obtner apuntador a elemento siguiente
      straw = &( *(straw->insert(s[i],StrType::none)) ) ;
      
    }
    if(s.size() > 0){
      straw->insert(s[s.size()-1],type,idx);
    }
  }
  
  //Salida estandar de la estructura del arbol. ! significa nodo terminal.
  void debug() const{
    std::cout << "[" << (strtype == StrType::none? "!" : "") << elem << "]{";
    for(const auto  &e : ptr){
      e.debug();
      std::cout << " ";
    }
    std::cout << "}";
  }
};

//Camina recursivante por el arbol trie y genera el código
void walkTree_r(std::ostream &out, std::list<TableEntry<char> > &stack){
  //Esta sección genera la etiqueta
  out << prefix;
  string casename;
  casename.reserve(stack.size());
  for(const auto &it : stack){
    casename += it.elem;
  }
  out << casename << ":\n";
  //Esta sección genera el código de condicion de escape cuando se encuentra un token
  if(stack.back().strtype != StrType::none){
    out << "\tif(!" << is_identifier << "("<< char_name<<") ){\n\t\t"
  << token_ready << "("<< classNb(stack.back().strtype) <<","<< stack.back().level <<")"<<";\n"
  "\t\tgoto " << default_case << ";\n\t}\n\t";
  }
  //Esta sección trada con los nodos hijo
  if(stack.back().ptr.empty()){
    out << "\t\tgoto "<< id_label << ";\n";
  }else{
    out << "\tswitch(" << char_name << "){\n";
    for(const auto &it: stack.back().ptr){
      out<<"\tcase '"<<it.elem<<"':\n"
      "\t\t" << save_char << "(" << char_name << ");\n"
      "\t\t" << request_char << "(" << char_name << ");\n"
      "\t\tgoto "<< prefix <<casename<<it.elem<<";\n"
      << "\t\tbreak;\n";
    }
    out << "\tdefault:\n"<<
    "\t\tgoto "<< id_label << ";\n"
    "\t}\n";
  }
  //Repite la operación con los nodos hijo
  for(auto &it: stack.back().ptr){
    stack.push_back(it);
    walkTree_r(out,stack);
    stack.pop_back();
  }
  
}
//Genera el códio completo de la máquina de estados
void walkTree_h(std::ostream &out, std::list<TableEntry<char> > &stack){
  //Genera el nodo de identificadores del usuario
  out << id_label << ":\n"
  "\tif(!" << is_identifier << "("<< char_name<<") ){\n\t\t"
  << token_ready << "("<<classNb(StrType::id)<<",0)" <<";\n"
  "\t\tgoto " << default_case << ";\n\t}\n\t"
  << save_char << "("<< char_name << ");\n\t"
  << request_char << "("<< char_name << ");\n"
  "\tgoto "<<id_label <<";\n"; 
  //Esta sección genera el estado inicial
  out << prefix;
  string casename;
  casename.reserve(stack.size());
  for(const auto &it : stack){
    casename += it.elem;
  }
  out << casename << ":\n" <<
  "\tswitch("<<char_name<<"){\n";
  for(const auto &it: stack.back().ptr){
    out<<"\tcase '"<<it.elem<<"':\n"
    "\t\t" << save_char << "(" << char_name << ");\n"
    "\t\t" << request_char << "(" << char_name << ");\n"
    "\t\tgoto "<< prefix <<casename<<it.elem<<";\n"
    << "\t\tbreak;\n";
  }
  out << "\tdefault:\n"<<
    "\t\tgoto "<<id_label<< ";\n"
   "\t}\n";
  for(auto &it: stack.back().ptr){
    stack.push_back(it);
    walkTree_r(out,stack);
    stack.pop_back();
  }
}
//Estado de la entrada (para eliminar espacio en blanco)
enum class ReadState{
  ID,
  SPACE,
  MOD1,
  MOD2,
  MOD3
};
int main(int argc, char **argv) {
    //Archivos de entrada/sailida
    std::istream *input;
    std::ostream *output;
    std::ifstream source_file;
    std::ofstream output_file;
    //Manejar linea de comandos
    if (argc > 3){
      std::cerr << "Numero incorrecto de argumentos";
      return -1;
    }
    if(argc < 2){
      input = &std::cin;
    }else{
      source_file.open( argv[1]);
      if(!source_file.is_open()){
	std::cerr << "Archivo " << argv[2] << " no se puede leer" << std::endl;
	return -1;
      }
      input = &source_file;
    }
    
    if(argc == 3){
      output_file.open( argv[2]);
      if(!output_file.is_open()){
	std::cerr << "Archivo " << argv[2] << " no se puede escribir" << std::endl;
	return -1;
      }
      output = &output_file;
    }else{
      output = &std::cout;
    }
    
    //El stack que se usa para recorrer el arbol trie
    TableEntry<char> head;
    head.elem = 'S';
    //Buffer de cadena recibida
    string buffer;
    int cur;
    ReadState state = ReadState::SPACE;
    
    //Saltar a la parte de declaracion de palabras a reconocer
    input->ignore(std::numeric_limits< int >::max(),'@');
    input->ignore(std::numeric_limits< int >::max(),'@');
    
    //Si llegamos al final, el archivo tenia mal formato
    if(!input->good()){
      return - 1;
    }
    
    //Contador de cada tipo de palabra reconocida
    unsigned i[4] = {0,0,0,0};
    while(1){
      
      cur = input->get();
      
      if(cur < 1){
          //TODO: manejar los otros sufijos
	if(state == ReadState::ID){
	  head.pushString(buffer,StrType::keyword,i[0]++);
	}
	break;
      }
      //Distinguir cadenas según sufijo
      if(state == ReadState::ID){
	if(std::isspace(cur)){
	  head.pushString(buffer,StrType::keyword,i[0]++);
	  buffer.clear();
	  state = ReadState::SPACE;
	}else if(cur == '$'){
	  head.pushString(buffer,StrType::type_builtin,i[1]++);
	  buffer.clear();
	  state = ReadState::SPACE;
	}else if(cur == '!'){
	  head.pushString(buffer,StrType::proc_builtin,i[2]++);
	  buffer.clear();
	  state = ReadState::SPACE;
	}else if(cur == '#'){
	  head.pushString(buffer,StrType::fun_builtin,i[3]++);
	  buffer.clear();
	  state = ReadState::SPACE;
	}else{
	  buffer += cur;
	}
      }
      if(state == ReadState::SPACE){
	if(!(std::isspace(cur) || cur == '#' || cur == '!'|| cur == '$')){
	  buffer += cur;
	  state = ReadState::ID;
	}
      }
      
      //head.pushString(buffer.substr(i,buffer.size()-1));
    }
    
    //TODO: esto no funciona en linea de comandos
    if(source_file.is_open()){
      source_file.clear();
      source_file.seekg(0, std::istream::beg);
    }
    //Copiar todo el achivo hasta la aparición del símboolo @
    while((cur = input->get()) > 0 && cur != '@'){
      output->put(cur);
    }
    
    //head.debug();
    
    std::list< TableEntry<char> > stack;
    stack.push_back(head);
    
    //Generar el codigo
    walkTree_h(*output,stack);
    
    //Copiar el resto del archivo, hasta antes de el siguiente @
    while((cur = input->get()) > 0 && cur != '@'){
      output->put(cur);
    }
    
    return 0;
}
