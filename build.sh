#!/bin/bash
g++ -std=c++11 fsmm/main.cpp -o afsmm
./afsmm lexer.fs.cpp lexer.cpp
g++ -std=c++11 -Wall -I include main.cpp lexer.cpp lexer_base.cpp symtable.cpp data.cpp -o badpas
