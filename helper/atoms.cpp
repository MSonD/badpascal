#include <iostream>
#include <iomanip>
#include <string>
#include <map>
const std::string S_atoms [] = {
  "&", "y", "e", "\\", "z", "d", "o", "f", "p", "s", "%", "!", "|", "#", "@", "h", "j", "k", "v", "q",
  "a",
  "b","c","i","r", "↓",
  "¢",
  "g", "u","l", "t", "w", "m",
  "+","-","*","/",
  "<","_","¿","?",">","$",
  "(",")","[","]",",",";",":",".",
  "n",
  "x",
  "\"",
  "=",
  "⊢",
  "M",
  "N",
  "V",
  "H",
  "D'",
  "J",
  "G",
  "S",
  "A",
  "L",
  "W",
  "U",
  "Y",
  "X",
  "K",
  "P",
  "B",
  "I",
  "O",
  "R",
  "Q",
  "D",
  "Z",
  "E",
  "E'",
  "T",
  "T'",
  "F",
  "C",
  "R'"
};


int main(){
	std::map<std::string,unsigned> map;
	for(unsigned i = 0; i < (sizeof(S_atoms)/sizeof(std::string));i++){
		map[S_atoms[i]] = i;
	}
	std::string nb, rule, value, segment;
	size_t pos = 0;
	while(std::getline(std::cin, nb, '\t')) {
		std::cout << std::hex << std::showbase <<std::internal
			   << nb << "\t";
		std::getline(std::cin, rule, ' ');
		std::cout << map[rule];
		std::getline(std::cin, rule, ' ');
		std::cout << rule;
		std::getline(std::cin, rule,  '\t');
		std::cout << rule << "\t";
		while ((pos = rule.find(" ")) != std::string::npos) {
			segment = rule.substr(0, pos);
			std::cout <<std::setfill('0') << std::setw(4)   << map[segment] << " ";
			rule.erase(0, pos + 1);
		}
		std::cout  <<std::setfill('0') << std::setw(4)  << map[ rule] << "\t";
		std::getline(std::cin, value, '\n');
		while ((pos = value.find(" ")) != std::string::npos) {
			segment = value.substr(0, pos);
			std::cout  <<std::setfill('0') << std::setw(4)  << map[segment] << " ";
			value.erase(0, pos + 1);
		}
		std::cout  <<std::setfill('0') << std::setw(4)  << map[ value];
		std::cout << std::endl;
	}
}
