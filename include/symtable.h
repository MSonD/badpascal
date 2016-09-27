#ifndef PAS_SYMTABLE_H
#define PAS_SYMTABLE_H
#include <string>
#include "data.h"
#include "lexer_base.h"


/* class Vector
{
public:
  T& operator[](unsigned index)
  {
    return impl[index];
  }
  unsigned push_back(T&& in)
  {
      impl.push_back(std::move(in));
      return impl.size();
  }
  size_t size()
  {
    return impl.size();
  }
private:
  std::vector<T> impl; //TODO: REPLACE
};
*/
class TableEntry{
public:
  std::string string;
  unsigned pos;
  unsigned type;
  TableEntry(const std::string& str,unsigned pos);
};
class Context{
public:
  Hash<std::string,unsigned> ids;
  size_t ids_i = 0;
  Hash<std::string,unsigned> strs;
  size_t strs_i = 0;
};
class TokenPusher{
public:
  //Creadores de token
  virtual void String(const std::string& element, Context &tables) = 0;
  virtual void ID(const std::string& element, Context &tables) = 0;
  virtual void Language(unsigned type,long id) = 0;
  virtual void Integer(const std::string& str, int base = 10) = 0;
  virtual void Float(const std::string&) = 0;
  virtual void Special(unsigned type, unsigned subtype) = 0;
  void AutoString (unsigned type, long id, const std::string& string, Context &tables);
  virtual void Endl() = 0;
};

#endif // SYMTABLE_H
