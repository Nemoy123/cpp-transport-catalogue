#pragma once
#include <iostream>
#include "transport_catalogue.h"



using namespace std::literals;


namespace transcat::input {

void Load(transcat::TransportCatalogue& cat, std::istream& input);

} //конец namespace Transcat