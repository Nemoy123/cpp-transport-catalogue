#pragma once
#include <iostream>
#include <map>
#include <string>

namespace transcat::output {

std::ostream& DateOutput (std::ostream& output_cout);
std::ostream& DateOutput (std::ostream& output_cout, std::string& in);
std::ostream& DateOutput (std::ostream& output_cout, std::tuple <std::string_view, std::size_t, std::size_t, double, double>& bus);
std::ostream& DateOutput (std::ostream& output_cout, std::map <int, std::string>& stop);


} // конец namespace