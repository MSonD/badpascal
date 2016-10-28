#include "parse_base.h"
bool Parser::step()
{ 
  size_t index = head+stack.peek()*width;
  assert(stack.size()  > 0);
  assert(index < table.size());
  unsigned char flags = table[index].flags;
  if(flags & Entry::STOP){
    head = source->fetch();
  }
  if(flags & Entry::POP){
    stack.pop_back();
  }
  if(flags & Entry::REPLACE){
    stack.pop_back();
    pushString(replace_strings[table[index].replace]);
  }
  if(flags & Entry::FINISH || head == static_cast<unsigned>(I_INDEXES::FEOF)){
    
    return false;
  }
  if(flags == 0){
    head = source->fetch();
    //TODO:ERROR
  }
  return true;
}

void Parser::operator()()
{
  while(step()){}
  
}

void Parser::pushString(const std::string& string)
{
  for(unsigned i = string.size(); i != 0 ;i--){
    stack.push_back(i-1);
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