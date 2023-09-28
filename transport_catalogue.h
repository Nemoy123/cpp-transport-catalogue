#pragma once
#include <iostream>
#include <deque>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "geo.h"

namespace transcat {

class TransportCatalogue {
public:
    struct Stop;
    struct Bus;
    struct Stop {
        std::string name;
        Coordinates xy;
    };

    struct Bus {
        std::string name;
        std::deque <const Stop*> bus_stops;
        bool ring;
    };
    void AddStop (const Stop& stop);
    void AddBusRoute (const Bus& bus);
    const Stop* FindStop (const std::string_view stop);
    const Bus* FindBus (const std::string_view bus);
    std::deque <Stop>& GiveMeStops () {return all_stops_;}
    void GetBusInfo (std::ostream &s, const std::string_view bus_name, const Bus* bus);
    void GetStopInfo (std::ostream &s, const std::string_view stop_name, const Stop* stop);
    void InputDistance (const Stop* stopA, const Stop* stopB, const int distance);
    double GetDistance (const Stop* stopA, const Stop* stopB);
    std::unordered_map <std::string_view, const Stop*> GetMap() const {return map_stops_;}
    
private:
    std::deque <Stop> all_stops_;
    std::deque <Bus> all_buses_;
    std::unordered_map <std::string_view, const Stop*> map_stops_;
    std::unordered_map <std::string_view, const Bus*> map_buses_;
    std::unordered_map <const Stop*, std::unordered_set <const Bus*>> buses_for_stop_;
    
    struct Hash {
        size_t operator() (const std::pair<const Stop*, const Stop*>& doom) const {
            size_t x =  (size_t)doom.first;
            size_t y = (size_t)doom.second;
            return x*(17)  + y * (17*17*17);
        }
    };
    std::unordered_map <std::pair<const Stop*, const Stop*>, int, Hash> distance_stops;
};

} //конец namespace Transcat