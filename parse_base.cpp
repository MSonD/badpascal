#include "parse_base.h"
void Parser::step()
{ 
  size_t index = head+stack.peek()*width;
  unsigned char flags = table[index].flags;
  if(flags == 0){
    //error
  }
  if(flags & !Entry::STOP){
    head = source->fetch();
  }
}
void Parser::setSource(AtomSource& src)
{
  source = &src;
}

