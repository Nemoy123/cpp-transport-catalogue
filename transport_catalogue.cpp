#include "transport_catalogue.h"
#include <set>
#include <map>

using namespace std;

namespace transcat {

const Stop* TransportCatalogue::FindStop (const std::string_view stop) {
   const auto iter = map_stops_.find(stop);
    return iter != map_stops_.end() ? iter->second : nullptr;
}

const Bus* TransportCatalogue::FindBus (const std::string_view bus){
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

BusInfo TransportCatalogue::GetBusInfo (const std::string_view bus_name) {
    auto bus = FindBus (bus_name);
    //std::tuple <std::string_view, size_t, size_t, double, double> result;
    BusInfo result;
    if (bus == nullptr) {
        return {bus_name, 0, 0, 0, 0};
    }
    else {
        
        size_t count = (bus->bus_stops.size());
        size_t uniq = std::set <const Stop*> ( bus->bus_stops.begin(), bus->bus_stops.end() ).size();
        double dist = 0;
        double real_distance = 0;
            
            for (size_t i = 0; i+1 < count; ++i) {
                
                    dist += ComputeDistance (bus->bus_stops[i]->xy, bus->bus_stops[i + 1]->xy);
                    real_distance += GetDistance (bus->bus_stops[i], bus->bus_stops[i + 1]);
            }
        double curv = real_distance/dist;
        return {bus_name, count, uniq, real_distance, curv};
    }
}
std::map <int, std::string> TransportCatalogue::GetStopInfo (const std::string_view stop_name) {

    const auto stop = FindStop (stop_name);
    std::string name {stop_name};
    if (stop == nullptr) {
        std::string not_find = "not found"s;
        return {{0, name}, {1, not_find}};
    }
    else {
        if (buses_for_stop_[stop].size() > 0) {

            std::map <int, string> result = {{0, name}, {1,{}}};
            std::set <std::string> temp_set;
            int i = 2;
            for (const auto bus : buses_for_stop_[stop]) {
                temp_set.insert(bus->name);
            }

            for (const auto bus_name : temp_set) {
                result.insert({i, bus_name});
                ++i;
            }
            
            return result;
        }
        else {
            std::string no_bus = "no buses"s;
            return {{0, name}, {1, no_bus}};
        }
    }
}

double TransportCatalogue::GetDistance (const Stop* stopA, const Stop* stopB) const {
    if (distance_stops.find({stopA, stopB}) == distance_stops.end())
        { return static_cast<double> (distance_stops.at({stopB, stopA})); }
    return static_cast<double> (distance_stops.at({stopA, stopB}));
}

void TransportCatalogue::InputDistance (const Stop* stopA, const Stop* stopB, const int distance) {
    distance_stops[{stopA, stopB}] = distance;
}



} // конец namespace