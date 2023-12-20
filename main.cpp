#include <fstream>
#include <iostream>
#include <string_view>
#include <string>
#include <iostream>
#include <sstream>
#include "request_handler.h"
#include <fstream>
// #include "log_duration.h"

//using namespace transcat::input;
using namespace transcat;
using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);
       std::ostream& output = std::cout ;
       std::istream& input = std::cin;
    if (mode == "make_base"sv) {

        request::MakeBase (input);

    } else if (mode == "process_requests"sv) {

        request::ProcessRequests (input, output);

    } else {
        PrintUsage();
        return 1;
    }
}

// int main (){
//     std::ofstream output("test3_out.json");
//     std::ifstream input1("test2.json");
//     std::ifstream input2("test3.json");

//     request::MakeBase (input1);
//     request::ProcessRequests (input2, output);
// //     request::LoadInput(input, output);
//  }

