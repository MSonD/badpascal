#include "data.h"
template <>
hash_type hash_fun<std::string>(std::string string){
  hash_type acc = 0;
  for(auto &c : string){
    acc +=  (c<<13)-1;
  }
  return acc;
}