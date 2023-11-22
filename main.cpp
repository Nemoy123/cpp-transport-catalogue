#include <iostream>
#include <sstream>
#include "request_handler.h"
#include <fstream>
#include "log_duration.h"

//using namespace transcat::input;
using namespace transcat;


int main() {
    
// std::ofstream output("out21.json");
// std::ifstream input("in_test_21.json");
// request::LoadInput(input, output);
// {
//     LogDuration test_time ("Test_e4"s); 
//     std::ofstream output("out_e4.json");
//     std::ifstream input("e4_input.json");
//     request::LoadInput(input, output);
// // }
  
 request::LoadInput(std::cin, std::cout);

}