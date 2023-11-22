#include "json_reader.h"
#include "json_builder.h"
#include <deque>
#include <string>
#include <unordered_map>
#include <algorithm>
#include "domain.h"
#include "log_duration.h"

using namespace std::literals;
using namespace json;
namespace json::read {

void JSONReader::FillStop (  std::unordered_map <std::string, std::set<std::pair <std::string, int>>>& waiting_stop_distance, 
                const Node& element_map ) {
       Stop stop;
        auto it = element_map.AsDict().find("name"s);
        if (it != element_map.AsDict().end()) {
            stop.name = it->second.AsString();
        }
        it = element_map.AsDict().find("latitude"s);
        if (it != element_map.AsDict().end()) {
            stop.xy.lat = it->second.AsDouble();
        }
        it = element_map.AsDict().find("longitude"s);
        if (it != element_map.AsDict().end()) {
            stop.xy.lng = it->second.AsDouble();
        }
        //добавить остановку перед проверкой дистанций
        cat_.AddStop(stop);

        it = element_map.AsDict().find("road_distances"s);
        if (it != element_map.AsDict().end()) {
            //std::unordered_map <std::pair<const Stop*, const Stop*>, int, Hash> distance_stops;
            
            //проверить наличие остановок, если что запихнуть в очередь
            auto map_dist = it->second.AsDict();
            for (const auto& [name_d, dist] : map_dist){
                    if (cat_.FindStop(name_d) != nullptr) {
                        cat_.InputDistance(cat_.FindStop(stop.name), cat_.FindStop(name_d), dist.AsInt());
                    }
                    else {
                        
                        //waiting_stop_distance.insert ({stop.name, {name_d, dist.AsInt()}});
                        waiting_stop_distance[stop.name].insert ({name_d, dist.AsInt()});
                    }
            }
        }

}

void JSONReader::FillBus (const Node& element_map){
    Bus bus;
    auto it = element_map.AsDict().find("name"s);
    if (it != element_map.AsDict().end()) {
        bus.name = it->second.AsString();
    }
     
     it = element_map.AsDict().find("is_roundtrip"s);
    if (it != element_map.AsDict().end()) {
         bus.ring = it->second.AsBool();
    }

    it = element_map.AsDict().find("stops"s);
    if (it != element_map.AsDict().end()) {
        auto array = it->second.AsArray();
        if (!bus.ring) {
            std::deque<Node> temp {array.rbegin(), array.rend()};
            temp.pop_front();

            for (auto& stop : array) {
                bus.bus_stops.push_back(cat_.FindStop(stop.AsString()));
            }
            for (auto& stop : temp) {
                bus.bus_stops.push_back(cat_.FindStop(stop.AsString()));
            }
        }
        else {
            for (auto& stop : array) {
                bus.bus_stops.push_back(cat_.FindStop(stop.AsString()));
            }
        }
    }
    cat_.AddBusRoute(bus);
}


void JSONReader::FillCatalogue (const std::deque <Document>& documents) {
    std::unordered_map <std::string, std::set<std::pair <std::string, int>>> waiting_stop_distance;
    for (const auto& doc: documents) {
        const json::Node* temp1 = &doc.GetRoot();
        bool test_clear_buses = false;
        if (!temp1->IsDict()) {
            
            std::cout << "Find error \n";
            auto test_array = temp1->AsArray();
            for ( auto& elem : test_array) {
                auto test2 = elem.AsDict();
                auto bus_clear = test2.find("buses"s);
                if ( bus_clear != test2.end()) {
                    test_clear_buses = true; // пропусть дальнейшее
                }
            }
            
        } 
        if (test_clear_buses)  {continue;}
        auto map_upper_level = (temp1->AsDict());
        auto it_map_ul = (map_upper_level).find("base_requests"s);
        if (it_map_ul == (map_upper_level).end()) { return;}
        auto map_base_requests = (it_map_ul->second).AsArray();
        
        
        for (const auto& element_map  : map_base_requests)  {
             auto it = element_map.AsDict().find("type"s);
            if (it != element_map.AsDict().end()) {
                std::string stop_check = it->second.AsString();
                if (stop_check == "Stop"s){
                   // std::cout << "Stop found"<< std::endl;
                    FillStop (waiting_stop_distance, element_map);
                }
            }
        }
        int all_stop_add = 0;
        do  {
                all_stop_add = 0;
                for (auto& stop_w : waiting_stop_distance) {
                    for (auto& [stop1, stop2_d] : stop_w.second) {
                        auto stop1_uk = cat_.FindStop(stop_w.first);
                        auto stop2_uk = cat_.FindStop(stop1);
                        if (stop1_uk != nullptr && stop2_uk != nullptr) {
                            cat_.InputDistance(stop1_uk, stop2_uk, stop2_d);
                        }
                        else {++all_stop_add;}
                    }
                }
        } while (all_stop_add != 0);

        for (const auto& element_map  : map_base_requests)  {
             auto it = element_map.AsDict().find("type"s);
            if (it != element_map.AsDict().end()) {
                std::string stop_check = it->second.AsString();
                if (stop_check == "Bus"s) {
                    //std::cout << "Bus found"<< std::endl;
                    FillBus (element_map);    
                }
            }
        }
    }
}

std::deque <request::RequestHandler::Request> JSONReader::ReadRequest ( std::deque <Document>&& documents) {
    std::deque <request::RequestHandler::Request> result;
    for (auto& doc: documents) {
        const json::Node* temp1 = &doc.GetRoot();
        if (!temp1->IsDict()) {
            std::cout << "не валидный запрос\n";
        }
        auto map_upper_level = (temp1->AsDict());
        auto it_map_ul = (map_upper_level).find("stat_requests"s);
        if (it_map_ul == (map_upper_level).end()) { return {};}
        auto array_base_requests = (it_map_ul->second).AsArray();
        for (const auto& map_req  : array_base_requests)  {
           request::RequestHandler::Request req_el;
           auto it = map_req.AsDict().find("id");
           if (it != map_req.AsDict().end()) {
                req_el.id  = it->second.AsInt();
                
           }
            it = map_req.AsDict().find("type");
           if (it != map_req.AsDict().end()) {
                req_el.type = it->second.AsString();
           }
            it = map_req.AsDict().find("name");
            if (it != map_req.AsDict().end()) {
                req_el.name = it->second.AsString();
           }
           it = map_req.AsDict().find("from");
            if (it != map_req.AsDict().end()) {
                req_el.from_stop = it->second.AsString();
           }
           it = map_req.AsDict().find("to");
            if (it != map_req.AsDict().end()) {
                req_el.to_stop = it->second.AsString();
           }
           result.push_back(req_el);
        }

     
    }
    return result;
}

std::string JSONReader::MakeRGB (int red, int green, int blue) {
        std::stringstream sstm;
        sstm << "rgb("sv << static_cast <int> (red) << ","sv << static_cast <int> (green) << ","sv<< static_cast <int> (blue) << ")"sv;
         return {sstm.str()};
}

std::string JSONReader::MakeRGBA (int red, int green, int blue, double opacity) {
        std::stringstream sstm;
        sstm << "rgba("sv << static_cast <int> (red) << ","sv 
             << static_cast <int> (green) << ","sv<< static_cast <int> (blue) 
             << ","sv << static_cast <double> (opacity)  << ")"sv;
        return {sstm.str()};
}

std::string JSONReader::MakeColor (json::Node color) {
    if (color.IsString()) {
         return color.AsString();
    }
    else if (color.IsArray()) {
         std::deque <int> col;
            double opa =1.0;
            for (auto& rgb_rgba : color.AsArray()) {
                if (rgb_rgba.IsInt()) {
                    col.push_back(rgb_rgba.AsInt());
                } 
                else  { 
                    opa = rgb_rgba.AsDouble(); 
                    return MakeRGBA(col[0], col[1], col[2], opa); 
                }
            }
            
            if (color.AsArray().size() == 3 ) {
                 return MakeRGB(col[0], col[1], col[2]);
            }
    }
     return {}; 
} 


void JSONReader::ReadRenderSettings (const std::deque <Document>& raw_documents) {
    RenderSettings render_settings;
    for (auto& doc: raw_documents) {
        const json::Node* temp1 = &doc.GetRoot();
        if (!temp1->IsDict()) {
            std::cout << "не валидный запрос\n";
        }
        auto map_upper_level = (temp1->AsDict());
        auto it_map_ul = (map_upper_level).find("render_settings"s);
        if (it_map_ul == (map_upper_level).end()) { return;}
        auto map_base_render = (it_map_ul->second).AsDict();
        for (const auto& [name, second] : map_base_render)  {
            if      (name == "width"s) { render_settings.width = second.AsDouble(); }
            else if (name == "height"s) { render_settings.height = second.AsDouble(); }
            else if (name == "padding"s) { render_settings.padding = second.AsDouble(); }
            else if (name == "line_width"s) { render_settings.line_width = second.AsDouble(); }
            else if (name == "stop_radius"s) { render_settings.stop_radius = second.AsDouble(); }
            else if (name == "bus_label_font_size"s) { render_settings.bus_label_font_size = second.AsInt(); }
            else if (name == "bus_label_offset"s) {
               render_settings.bus_label_offset.dx = second.AsArray().at(0).AsDouble();
               render_settings.bus_label_offset.dy = second.AsArray().at(1).AsDouble();
            }
            else if (name == "stop_label_font_size"s) { render_settings.stop_label_font_size = second.AsInt(); }
            else if (name == "stop_label_offset"s) {
               render_settings.stop_label_offset.dx = second.AsArray().at(0).AsDouble();
               render_settings.stop_label_offset.dy = second.AsArray().at(1).AsDouble();     
            }
            else if (name == "underlayer_color"s) {
                render_settings.underlayer_color = MakeColor (second);
            }
            else if (name == "underlayer_width"s) { render_settings.underlayer_width = second.AsDouble(); }
            else if (name == "color_palette"s) {
                auto arr_color = second.AsArray();
                for (auto& color_temp : arr_color) {
                        render_settings.color_palette.color_palette.emplace_back(MakeColor (color_temp));
                }

            }

            else {std::cout << "ERROR render_settings"; }

        }

    }
    render_settings_ = (std::move (render_settings));
}
transcat::TransportCatalogue::RoutingSet JSONReader::ReadRoutingSettings (const std::deque <Document>& raw_documents) {
    transcat::TransportCatalogue::RoutingSet result; 
    for (auto& doc: raw_documents) {
        const json::Node* temp1 = &doc.GetRoot();
        if (!temp1->IsDict()) {
            std::cout << "не валидный запрос\n";
        }
        auto map_upper_level = (temp1->AsDict());
        auto it_map_ul = (map_upper_level).find("routing_settings"s);
        if (it_map_ul == (map_upper_level).end()) { return {};}
        auto map_base_render = (it_map_ul->second).AsDict();

        for (const auto& [name, second] : map_base_render)  {
            if      (name == "bus_velocity"s) { result.bus_velocity = second.AsInt(); }
            else if (name == "bus_wait_time"s) { result.bus_wait_time = second.AsDouble(); }   
        }
    }
    return result;
}

void json::read::JSONReader::LoadJSON () {
    
    std::deque <Document> raw_documents;
    
    while (input_.peek() != EOF) {
        
        Document temp_doc = json::Load(input_);
        if (!temp_doc.empty()) {raw_documents.push_back (temp_doc);}
        
    }
    // {
    //     LogDuration load1 ("load1 time "s);
        FillCatalogue (raw_documents);
    // }

    // {
    //     LogDuration load2 ("load2 time "s);
        ReadRenderSettings(raw_documents);
    // }
    // {
    //     LogDuration load3 ("load3 time "s);
        cat_.SetRoutingSet(ReadRoutingSettings(raw_documents));
    // }
    // {
    //     LogDuration load4 ("load4 time "s);
        requests_ = ReadRequest (std::move(raw_documents));
    //}
    
}

Dict json::read::JSONReader::FormatAnsertToJSON (request::RequestHandler::Answer& answer) {
    Dict result;
    Array bus_set;
    result.insert ({"request_id"s, Node (answer.id)});
    if (answer.type == "Stop"s) {
        if (!answer.not_found_buses && !answer.not_found_stop) {
            for (const auto& bus : answer.ans_set) {
                bus_set.push_back(Node(bus));
            }
            //Node temp_array (bus_set);
            result.insert ({"buses"s, Node (bus_set)});
        }
        
        else if (answer.not_found_buses) {
            //json::Node temp (" "s);
            result.insert ( { "buses"s, Node (bus_set) } );
        }
        // "error_message": "not found"
        else if (answer.not_found_stop) {
            result.insert ({"error_message"s, Node ("not found"s)});
        }
        
    }

    if (answer.type == "Bus"s) {
        if (answer.not_found_buses) {
            result.insert ({"error_message"s, Node ("not found"s)});
        }else {
            auto date = cat_.GetBusInfo (answer.name);
            //bus_name, count, uniq, real_distance, curv
            result.insert ( {"stop_count"s, Node ( static_cast<int> (date.count_stops) )} );
            result.insert ( {"unique_stop_count"s, Node ( static_cast<int> (date.uniq_stops) )} );
            result.insert ( {"route_length"s, Node ( date.real_dist )} );
            result.insert ( {"curvature"s, Node ( date.curv )} );
        }
    }
    if (answer.type == "Map"s) {
        auto docum = face_.RenderMap();
        std::stringstream map_output;
        docum.Render(map_output);
        
        result.insert ( { "map"s, Node {map_output.str()} } );
    }
    return result;
}

Node json::read::JSONReader::FormatAnsertToJSONBuilder (request::RequestHandler::Answer& answer) {
    //Dict result;
   // Array bus_set;
    Builder builder;
    //result.insert ({"request_id"s, Node (answer.id)});
    builder.StartDict().Key("request_id"s).Value(answer.id);

    if (answer.type == "Stop"s) {
        if (!answer.not_found_buses && !answer.not_found_stop) {
            builder.Key("buses"s);
            builder.StartArray();
            for (const auto& bus : answer.ans_set) {
                //bus_set.push_back(Node(bus));
                builder.Value(bus);
            }
            builder.EndArray();
            //result.insert ({"buses"s, Node (bus_set)});
            
        }
        
        else if (answer.not_found_buses) {
            builder.Key("buses"s).StartArray().EndArray();
            //result.insert ( { "buses"s, Node (bus_set) } );
        }
        // "error_message": "not found"
        else if (answer.not_found_stop) {
            builder.Key("error_message"s).Value("not found"s);
            //result.insert ({"error_message"s, Node ("not found"s)});
        }
        
    }

    if (answer.type == "Bus"s) {
        if (answer.not_found_buses) {
            builder.Key("error_message"s).Value("not found"s);
            //result.insert ({"error_message"s, Node ("not found"s)});
        }else {
            auto date = cat_.GetBusInfo (answer.name);
            //bus_name, count, uniq, real_distance, curv
            builder.Key("stop_count"s).Value(static_cast<int> (date.count_stops));
            builder.Key("unique_stop_count"s).Value(static_cast<int> (date.uniq_stops));
            builder.Key("route_length"s).Value(date.real_dist);
            builder.Key("curvature"s).Value(date.curv);
            // result.insert ( {"stop_count"s, Node ( static_cast<int> (date.count_stops) )} );
            // result.insert ( {"unique_stop_count"s, Node ( static_cast<int> (date.uniq_stops) )} );
            // result.insert ( {"route_length"s, Node ( date.real_dist )} );
            // result.insert ( {"curvature"s, Node ( date.curv )} );
        }
    }
    if (answer.type == "Map"s) {
        auto docum = face_.RenderMap();
        std::stringstream map_output;
        docum.Render(map_output);
        builder.Key("map"s).Value(map_output.str());
        //result.insert ( { "map"s, Node {map_output.str()} } );
    }
    if (answer.type == "Route"s) {
        if (answer.not_found_route) {
            builder.Key("error_message"s).Value("not found"s);
        }
        else {
            builder.Key("total_time"s).Value(answer.route_date.weight);
            builder.Key("items"s).StartArray();
            //size_t counter = 0;
            for (const auto& tr: answer.route_date.edges) {

                builder.StartDict();
                builder.Key("type"s).Value("Wait"s);
                //builder.Key("stop_name"s).Value(cat_.GetAllStops().at((answer.edge_info.at(tr).edge.from)).name);
                builder.Key("stop_name"s).Value(this->face_.GetGraph().GetEdge(tr).name_stop_from);
                builder.Key("time"s).Value(cat_.GetRoutingSet().bus_wait_time);
                builder.EndDict();

                builder.StartDict();
                builder.Key("type"s).Value("Bus"s);
                builder.Key("bus"s).Value(this->face_.GetGraph().GetEdge(tr).bus_name);
                //builder.Key("span_count"s).Value(static_cast <double> (answer.edge_info.at(tr).previous_segments_in_edge.size() + 1));
                //builder.Key("span_count"s).Value(static_cast <double> (answer.edge_info.at(tr).span_count + 1));
                builder.Key("span_count"s).Value(static_cast <double> (this->face_.GetGraph().GetEdge(tr).segment_edge_size + 1));
                builder.Key("time"s).Value(this->face_.GetGraph().GetEdge(tr).weight - cat_.GetRoutingSet().bus_wait_time);
                builder.EndDict();
                //++counter;
            } 

            builder.EndArray();
        }
    }


    builder.EndDict();
    auto result = builder.Build();
    return result;
}


} // namespace end