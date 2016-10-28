#include "parse_base.h"
#include <iostream>
#include <bitset>

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
    return false;
  }
  
  if(flags == 0){
    std::cerr << "ERROR linea " << source->line() << " Atómo " << S_atoms[stack.peek()] << " no esperaba " << S_atoms[head] << std::endl;
    head = source->fetch();
#ifndef NDEBUG
    printString(stack);
    std::cerr << std::endl;
    //TODO: ERROR HANDLING
#endif
  }
  return true;
}

void Parser::operator()()
{
  head = source->fetch();
  while(step()){}
  
}

void Parser::pushString(const std::string& string)
{
  for(unsigned i = string.size(); i != 0 ;i--){
    stack.push_back(string[i-1]);
  }
}

void Parser::setSource(AtomSource& src)
{
  source = &src;
}

Entry::Entry(unsigned char falags, unsigned char reaplace): flags(falags), replace(reaplace)
{
}
Entry::Entry() : Entry(0,0){}