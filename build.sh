#!/bin/bash
g++ -std=c++11 fsmm/main.cpp -o afsmm
./afsmm lexer.fs.cpp lexer.cpp
g++ -std=c++11 -Wall -DNDEBUG -I include main.cpp lexer.cpp lexer_base.cpp symtable.cpp data.cpp parse_base.cpp parser.cpp -o badpas -lpthread
