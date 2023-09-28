#include <iostream>
#include <sstream>
#include "transport_catalogue.h"
#include "input_reader.h"
#include <fstream>

using namespace transcat::input;
using namespace transcat;

int main() {

    TransportCatalogue catalog;
//  std::ifstream input("tsA_case1_input.txt");
//  Load (catalog, input, std::cout);
    Load (catalog, std::cin, std::cout);
}