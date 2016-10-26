#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <algorithm>
#include <atomic>
#include "lexer.h"
#include "symtable.h"
#include "parse_base.h"
#define TEST_MODE
#include "main.cpp"

int main(int argc, char** argv){
  Context context;
  //Consumidor de tokens
  std::cout << sizeof(Entry);
  TokenTester default_out;
  main0(argc,argv,context,default_out);
  //for(auto &e : default_out.
}

int test0(){
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
int test1(){
  Hash<std::string,unsigned> fun;
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
