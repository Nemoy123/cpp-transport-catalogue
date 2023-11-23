#include "request_handler.h"
#include <iostream>
#include <algorithm>
#include <execution>
#include <mutex>
#include <unordered_set>
#include "json_reader.h"
#include "log_duration.h"
#include "router.h"

using namespace std::literals;
using namespace renderer;
using namespace svg;
using namespace transcat;
using namespace json;


namespace request {

// void RequestHandler::YouCanMakeItRealRouter () { //const Request& it
//       //LogDuration router_graph_construction ("router_graph_construction "s); 

//                 //setting_router = true; 
//                 dw_graph_.SetVertexCount(db_.GetAllStops().size());
                
//                 for (const auto& [bus_name, bus] : db_.GetRoutes()) {
                
//                     const std::deque<const Stop*>& bus_stops = bus->bus_stops;
//                     std::vector <graph::Edge<double>> segment_edge(bus_stops.size());                
//                     for (auto i = bus_stops.cbegin(); i+1 != bus_stops.cend(); ++i) {
//                         segment_edge.clear();
//                         for (auto n = i+1; n != bus_stops.cend(); ++n) {
                                
//                                 graph::Edge<double> edge{};

                                
//                                 edge.bus_name = bus_name;

//                                 auto it_from = std::find ((db_.GetAllStops()).cbegin(), (db_.GetAllStops()).cend(), *(*i)); //std::execution::par, 
//                                 edge.from = it_from - db_.GetAllStops().cbegin();
//                                 edge.name_stop_from = (db_.GetAllStops().at(edge.from)).name;
//                                 auto it_to = std::find ( (db_.GetAllStops()).cbegin(), (db_.GetAllStops()).cend(), *(*n)); //std::execution::par,
//                                 edge.to = it_to - db_.GetAllStops().cbegin();
                                
                                
//                                 if (segment_edge.empty()) {
//                                     edge.weight = ((db_.GetDistance (*i, *n))/(db_.GetRoutingSet().bus_velocity*1000/60)) + db_.GetRoutingSet().bus_wait_time;
//                                 }
//                                 else {
//                                     edge.weight += segment_edge.back().weight;
//                                     const Stop* prev_stop = &(db_.GetAllStops())[segment_edge.back().to]; // последняя остановка в цепочке от нее считаем новое ребро
//                                     edge.weight += (db_.GetDistance (prev_stop, *n))/(db_.GetRoutingSet().bus_velocity*1000/60);
//                                 }
                                
//                                 edge.segment_edge_size = segment_edge.size();
//                                 dw_graph_.AddEdge(edge);
                                

//                                 segment_edge.push_back(std::move(edge));
//                         }
//                     }


//                 }
                
//                 router_ = (std::move(graph::Router<double> (dw_graph_)));
                
// }

void RequestHandler::ExecuteRequests () {
    //std::for_each( std::execution::par, req_deq_.begin(), req_deq_.end(), [&](auto& it) {
            for (auto& it : req_deq_) {
                Answer answer;
                answer.id = it.id;
                answer.type = it.type;
                
                if ((it).type == "Stop"s) {
                    answer.name = it.name;
                    
                    auto map_temp = db_.GetStopInfo((it).name); // ключ 0 имя стринг ключ 1 "not found" или "no buses"s, ключ 2 и далее стринг bus
                
                    if (map_temp[1] != "not found"sv && map_temp[1] != "no buses"sv) {
                        for (size_t i = 2; i < map_temp.size(); ++i) {
                            answer.ans_set.insert(map_temp[i]);    
                        }
                    }
                    else if (map_temp[1] == "not found"sv) {
                        answer.not_found_stop = true;
                    }
                    else if (map_temp[1] == "no buses"sv) {
                        answer.not_found_buses = true;
                    }

                }
                else if ((it).type == "Bus"s) {
                    answer.name = it.name;
                    auto bus_find = db_.FindBus((it).name);
                    if (bus_find == nullptr) {
                        answer.not_found_buses = true;
                    }
                    
                }
                else if ((it).type == "Route"s) {
                    
                    if (router_.GetGraph().GetEdgeCount() == 0 || router_.GetGraph().GetVertexCount() != db_.GetAllStops().size()) { 
                        //YouCanMakeItRealRouter ();
                        router_.YouCanMakeItRealRouter(db_);
                    }
                    
                    auto find_stop_from = db_.GetStopInfo(it.from_stop);
                    auto find_stop_to = db_.GetStopInfo(it.to_stop);
                    
                    if (find_stop_from.at(1) == "no buses"s || find_stop_to.at(1) == "no buses"s) {
                        answer.not_found_route = true;
                        answer_deq_.push_back(std::move(answer));
                        //return;
                        continue;
                    }
                
                    auto iter1 = std::find (std::execution::par, (db_.GetAllStops()).cbegin(),(db_.GetAllStops()).cend(), *(db_.FindStop(it.from_stop))); // 
                    size_t from_stop_size = iter1 - (db_.GetAllStops()).cbegin();
                    auto iter2 = std::find (std::execution::par, (db_.GetAllStops()).cbegin(), (db_.GetAllStops()).cend(), *(db_.FindStop(it.to_stop))); //
                    size_t to_stop_size = iter2 - (db_.GetAllStops()).cbegin();
                    
                    std::optional<graph::Router<double>::RouteInfo> route_info = router_.GetInRouter().BuildRoute(from_stop_size , to_stop_size);
                    if (route_info.has_value()) {
                        answer.route_date = std::move(route_info.value());
                    }
                    else {
                        answer.not_found_route = true;
                    }

                }
                
                answer_deq_.push_back(std::move(answer));
           }

//} );    
}   


template <typename T>
void OutputRun( T& json_reader, std::deque <request::RequestHandler::Answer>& answer_deq_, std::ostream& output_) {
    
    json::Array result;
    for (request::RequestHandler::Answer& ans : answer_deq_) {
        //json::Dict res = json_reader.FormatAnsertToJSON (ans);
        auto res = json_reader.FormatAnsertToJSONBuilder (ans);
        result.push_back(Node (res));
    }   

     json::PrintNode (result, output_);
}


svg::Document RequestHandler::RenderMap() const {
    MapRender map_render (render_settings_, db_);
    return map_render.MapRenderer();
}


void LoadInput (std::istream& input_t, std::ostream& output) {
    transcat::TransportCatalogue cat;
    request::RequestHandler face (cat, input_t, output);
    json::read::JSONReader json_reader (cat, face, input_t);
    // {
    //     LogDuration load ("load time "s);
        json_reader.LoadJSON();
    // }
    // {
    //     LogDuration input_request_deque ("input_request_deque time "s);
        face.InputRequestDeque(json_reader.GiveRequests());
    // }
    // {
    //     LogDuration input_render ("input_input_render time "s);
        face.InputRenderSettings(json_reader.GiveRenderSettings());
    // }
    // {
    //     LogDuration time_map_render ("input_map_render time "s);
        MapRender map_render(face.GetRenderSettings(), face.GetTransportBase());
    //}
    

    // { 
    //     LogDuration exec_requests ("exec_requests time "s);
        face.ExecuteRequests ();
    // }

    // { 
    //     LogDuration output_run ("output_run time "s); 
        OutputRun(json_reader, face.GetAnswerDeq(), face.GetOutput());
    // }
}  

} // end namespace 