#include "stat_reader.h"

using namespace std::literals;

namespace transcat::output {


std::ostream& DateOutput (std::ostream& output_cout) {
    return output_cout;
}

std::ostream& DateOutput (std::ostream& output_cout, std::string& in) {
    output_cout << in;
    return output_cout;
}
std::ostream& DateOutput (std::ostream& output_cout, std::tuple <std::string_view, std::size_t, std::size_t, double, double>& bus) {
    //std::ostream& out = std::cout;
    std::string_view name = std::get<0>(bus);
    std::size_t count = std::get<1>(bus);
    std::size_t uniq = std::get<2>(bus);
    double dis = std::get<3>(bus);
    double curv = std::get<4>(bus);
    if (count == 0 && uniq == 0 && dis == 0 && curv == 0) {
        output_cout << "Bus "s << name << ": not found"s << std::endl;
    }
     else {
         output_cout <<"Bus "s << name <<": "s << count 
         << " stops on route, "s << uniq << " unique stops, "s << dis << " route length, "s
           << curv <<" curvature"<< std::endl;
     }
    
    return output_cout;
}

std::ostream& DateOutput (std::ostream& output_cout, std::map <int, std::string>& stop) {
    //std::ostream& s = std::cout;
    output_cout <<"Stop "s << stop[0] <<": "s;
    if (stop.size() == 2) {
        output_cout << stop[1];
        output_cout << std::endl;
    }
    else {
        output_cout << "buses"s ;
        for (size_t i = 2; i < stop.size(); ++i) {
            output_cout << " "s << stop[i];
        }
        output_cout<< std::endl;
    }
    return output_cout;
}


}