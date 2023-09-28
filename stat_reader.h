#pragma once


namespace transcat::output {

std::ostream& DateOutput () {
    std::ostream& out = std::cout;
    return out;
}

std::ostream& DateOutput (std::string& in) {
    std::ostream& out = std::cout;
    out << in;
    return out;
}

std::ostream& DateOutput (std::tuple <std::string_view, std::size_t, std::size_t, double, double> bus) {
    std::ostream& out = std::cout;
    std::string_view name = std::get<0>(bus);
    std::size_t count = std::get<1>(bus);
    std::size_t uniq = std::get<2>(bus);
    double dis = std::get<3>(bus);
    double curv = std::get<4>(bus);
    if (count == 0 && uniq == 0 && dis == 0 && curv == 0) {
        out << "Bus "s << name << ": not found"s << std::endl;
    }
     else {
         out <<"Bus "s << name <<": "s << count 
         << " stops on route, "s << uniq << " unique stops, "s << dis << " route length, "s
           << curv <<" curvature"<< std::endl;
     }
    
    return out;
}

std::ostream& DateOutput (std::map <int, std::string> stop) {
    std::ostream& s = std::cout;
    s <<"Stop "s << stop[0] <<": "s;
    if (stop.size() == 2) {
        s << stop[1];
        s << std::endl;
    }
    else {
        s << "buses"s ;
        for (size_t i = 2; i < stop.size(); ++i) {
            s << " "s << stop[i];
        }
        s<< std::endl;
    }
    return s;
}


} // конец namespace