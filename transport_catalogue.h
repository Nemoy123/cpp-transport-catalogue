#pragma once

#include <iostream>
#include <deque>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <vector>
#include <map>
#include "geo.h"
#include "domain.h"

namespace transcat {

class TransportCatalogue {
public:
   
    void AddStop (const Stop& stop);
    void AddBusRoute (const Bus& bus);
    const Stop* FindStop (const std::string_view stop);
    const Bus* FindBus (const std::string_view bus);
    BusInfo GetBusInfo (const std::string_view bus_name);
    std::map <int, std::string> GetStopInfo (const std::string_view stop_name);
    void InputDistance (const Stop* stopA, const Stop* stopB, const int distance);
    double GetDistance (const Stop* stopA, const Stop* stopB);
    const std::unordered_map <std::string_view, const Stop*>& GetMap() const {return map_stops_;}
    const std::unordered_map <std::string_view, const Bus*>& GetRoutes() const {return map_buses_;}
    
    struct RoutingSet {
       int bus_wait_time = 0;
       double bus_velocity = 0.;
    };
    void SetRoutingSet(RoutingSet&& rhs) {routing_set_ = std::move(rhs);};
    const RoutingSet& GetRoutingSet () const {return routing_set_;}
    const std::deque <Stop>& GetAllStops () const {return all_stops_;}

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
    
    RoutingSet routing_set_;
};

} //конец namespace Transcat