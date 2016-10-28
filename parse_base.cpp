#include "parse_base.h"
#include <iostream>
#include <bitset>

//Un paso del parser
bool Parser::step()
{ 
  
  size_t index = head+stack.peek()*width;
  assert(stack.size()  > 0);
  assert(index < table.size());
  unsigned char flags = table[index].flags;
#ifndef NDEBUG
  std::cerr << "CELL "<< S_atoms[stack.peek()] << " :: "<<S_atoms[head]<< " " << std::bitset<8>(static_cast<unsigned>(table[index].flags)) << std::endl;
#endif
  if(flags & Entry::FORWARD){
#ifndef NDEBUG
    std::cerr << "FORWARD "  << std::endl;
#endif
    head = source->fetch();
  }
  
  if(flags & Entry::POP){
#ifndef NDEBUG
    std::cerr << "POP " << S_atoms[stack.peek()] << std::endl;
#endif
    stack.pop_back();
  }
  
  if(flags & Entry::REPLACE){
    stack.pop_back();
#ifndef NDEBUG
    std::cerr << "REPLACE ";
    printString(replace_strings[table[index].replace]);
    std::cerr << std::endl;
#endif
    pushString(replace_strings[table[index].replace]);
  }
  
  if(flags & Entry::FINISH || head == static_cast<unsigned>(I_INDEXES::FEOF)){
    if(stack.size() != 0){
      std::cerr << "ERROR se llego a fin de archivo sin completar estructura del programa" 
	"\n se esperaba " << stringOfAtom(stack.peek()) << std::endl;
      failure = true;
    }
    return false;
  }
  
  if(flags == 0){
    std::cerr << "ERROR linea " << source->line() << " Atómo " << stringOfAtom(stack.peek()) << " no esperaba " << stringOfAtom(head) << std::endl;
    failure = true;
    head = source->fetch();
#ifndef NDEBUG
    printString(stack);
    std::cerr << std::endl;
    //TODO: ERROR HANDLING
#endif
  }
  return true;
}

//Ejecuta el Parser
bool Parser::operator()()
{
  head = source->fetch();
  while(step()){}
  std::cerr << (failure? "NO ACEPTADO" : "ACEPTADO") << std::endl;
  return failure;
}
//Push una cadena de simbolos al stack
void Parser::pushString(const std::string& string)
{
  for(unsigned i = string.size(); i != 0 ;i--){
    stack.push_back(string[i-1]);
  }
}
//Cambiar origen de los atomos
void Parser::setSource(AtomSource& src)
{
  source = &src;
}

//Constructores del elelmento de la tabla
Entry::Entry(unsigned char falags, unsigned char reaplace): flags(falags), replace(reaplace)
{
}
Entry::Entry() : Entry(0,0){}