#include <iostream>
#include <sstream>
#include "request_handler.h"
#include <fstream>

//using namespace transcat::input;
using namespace transcat;

int main() {
    
//   std::ofstream output("output.json");
//   std::ifstream input("input.json");
//  request::LoadInput(input, output);
  
request::LoadInput(std::cin, std::cout);

}