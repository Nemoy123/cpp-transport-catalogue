#include "transport_catalogue.h"
#include <set>

using namespace std;

namespace transcat {

const TransportCatalogue::Stop* TransportCatalogue::FindStop (const std::string_view stop) {
   const auto iter = map_stops_.find(stop);
    return iter != map_stops_.end() ? iter->second : nullptr;
}

const TransportCatalogue::Bus* TransportCatalogue::FindBus (const std::string_view bus){
    const auto iter = map_buses_.find(bus);
    return iter != map_buses_.end() ? iter->second : nullptr;
}

void TransportCatalogue::AddStop (const Stop& stop) {
        all_stops_.push_back(stop);
        map_stops_.insert({all_stops_.back().name, &all_stops_.back()});
}

void TransportCatalogue::AddBusRoute (const Bus& bus) {
        
        all_buses_.emplace_back(bus);
        for (const Stop* stop : all_buses_.back().bus_stops) {
            buses_for_stop_[stop].insert (&all_buses_.back());
        }
        map_buses_.insert({all_buses_.back().name, &all_buses_.back()});
}

void TransportCatalogue::GetBusInfo (std::ostream &s, std::string_view bus_name, const Bus* bus) {
    
    if (bus == nullptr) {
        s <<"Bus "s << bus_name << ": not found"s << std::endl;
    }
    else {
        size_t count = bus->bus_stops.size();
        size_t uniq = std::set <const Stop*> ( bus->bus_stops.begin(), bus->bus_stops.end() ).size();
        double dist = 0;
        double real_distance = 0;
            for (size_t aborvalg = 0; aborvalg+1 < bus->bus_stops.size(); ++aborvalg) {
                
                    dist += ComputeDistance (bus->bus_stops[aborvalg]->xy, bus->bus_stops[aborvalg + 1]->xy);
                    real_distance += GetDistance (bus->bus_stops[aborvalg], bus->bus_stops[aborvalg + 1]);
            }
            
        double curv = real_distance/dist;
        s <<"Bus "s << bus_name <<": "s << count ;
        s << " stops on route, "s << uniq << " unique stops, "s << real_distance << " route length, "s
          << curv <<" curvature"<< std::endl;
    }
}

void TransportCatalogue::GetStopInfo (std::ostream &s, const std::string_view stop_name, const Stop* stop) {
    s <<"Stop "s << stop_name<<": "s;
    if (stop == nullptr) {
        s  << "not found"s << std::endl;
    }
    else {
        if (buses_for_stop_[stop].size() > 0) {
            s << "buses"s ;
            set <string_view> result;
            for (const auto bus : buses_for_stop_[stop]) {
                result.insert(bus->name);
            }
            for (const auto res : result) {
               s << " "s << res;
            }
            s<< std::endl;
        }
        else {
            s  << "no buses"s << std::endl;
        }
    }
}

double TransportCatalogue::GetDistance (const Stop* stopA, const Stop* stopB) {
    if (distance_stops.find({stopA, stopB}) == distance_stops.end())
        { return static_cast<double> (distance_stops[{stopB, stopA}]); }
    return static_cast<double> (distance_stops[{stopA, stopB}]);
}

void TransportCatalogue::InputDistance (const Stop* stopA, const Stop* stopB, const int distance) {
    distance_stops[{stopA, stopB}] = distance;
}



} // конец namespace