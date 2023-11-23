#pragma once
#include "router.h"
#include "transport_catalogue.h"

using namespace graph;
using namespace transcat;

template <typename Weight>
class TransportRouter {
    public:
        TransportRouter(): dw_graph_({}), inner_router_(dw_graph_){}
        void YouCanMakeItRealRouter (const TransportCatalogue& db_);
        graph::DirectedWeightedGraph<Weight>& GetGraph () {return dw_graph_;}
        graph::Router<Weight>& GetInRouter () {return inner_router_;}
    private:
        graph::DirectedWeightedGraph<Weight> dw_graph_{};
        graph::Router<Weight> inner_router_;
};



template <typename Weight>
void TransportRouter<Weight>::YouCanMakeItRealRouter (const TransportCatalogue& db_) { //const Request& it
      //LogDuration router_graph_construction ("router_graph_construction "s); 

                //setting_router = true; 
                dw_graph_.SetVertexCount(db_.GetAllStops().size());
                
                for (const auto& [bus_name, bus] : db_.GetRoutes()) {
                
                    const std::deque<const Stop*>& bus_stops = bus->bus_stops;
                    std::vector <graph::Edge<double>> segment_edge(bus_stops.size());                
                    for (auto i = bus_stops.cbegin(); i+1 != bus_stops.cend(); ++i) {
                        segment_edge.clear();
                        for (auto n = i+1; n != bus_stops.cend(); ++n) {
                                
                                graph::Edge<double> edge{};

                                
                                edge.bus_name = bus_name;

                                auto it_from = std::find ((db_.GetAllStops()).cbegin(), (db_.GetAllStops()).cend(), *(*i)); //std::execution::par, 
                                edge.from = it_from - db_.GetAllStops().cbegin();
                                edge.name_stop_from = (db_.GetAllStops().at(edge.from)).name;
                                auto it_to = std::find ( (db_.GetAllStops()).cbegin(), (db_.GetAllStops()).cend(), *(*n)); //std::execution::par,
                                edge.to = it_to - db_.GetAllStops().cbegin();
                                
                                
                                if (segment_edge.empty()) {
                                    edge.weight = ((db_.GetDistance (*i, *n))/(db_.GetRoutingSet().bus_velocity*1000/60)) + db_.GetRoutingSet().bus_wait_time;
                                }
                                else {
                                    edge.weight += segment_edge.back().weight;
                                    const Stop* prev_stop = &(db_.GetAllStops())[segment_edge.back().to]; // последняя остановка в цепочке от нее считаем новое ребро
                                    edge.weight += (db_.GetDistance (prev_stop, *n))/(db_.GetRoutingSet().bus_velocity*1000/60);
                                }
                                
                                edge.segment_edge_size = segment_edge.size();
                                dw_graph_.AddEdge(edge);
                                

                                segment_edge.push_back(std::move(edge));
                        }
                    }


                }
                
                inner_router_ = (std::move(graph::Router<double> (dw_graph_)));
                
}