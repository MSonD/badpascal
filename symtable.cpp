#include "symtable.h"
#include <utility>
#include <iostream>
//Constructor
TableEntry::TableEntry(const std::string& str, unsigned int position):string(str), pos(position)
{}
//Crea el token correcto de cadena con base al tipo
void TokenPusher::AutoString (unsigned type, long id, const std::string& string, Context &tables){
    if(type == Token::STRING){
      String(string,tables);
      return;
    }
    if(type == Token::ID){
      ID(string,tables);
      return;
    }
    Language(type,id);
}