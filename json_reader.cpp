#include "json_reader.h"
#include <deque>
#include <string>
#include <unordered_map>
#include <algorithm>

using namespace std::literals;
using namespace json;

void StopsCheck (transcat::TransportCatalogue& catalog, std::unordered_map <std::string, std::set<std::pair <std::string, int>>>& waiting_stop_distance, const Node& element_map){
        transcat::TransportCatalogue::Stop stop;
        auto it = element_map.AsMap().find("name"s);
        if (it != element_map.AsMap().end()) {
            stop.name = it->second.AsString();
        }
        it = element_map.AsMap().find("latitude"s);
        if (it != element_map.AsMap().end()) {
            stop.xy.lat = it->second.AsDouble();
        }
        it = element_map.AsMap().find("longitude"s);
        if (it != element_map.AsMap().end()) {
            stop.xy.lng = it->second.AsDouble();
        }
        //добавить остановку перед проверкой дистанций
        catalog.AddStop(stop);

        it = element_map.AsMap().find("road_distances"s);
        if (it != element_map.AsMap().end()) {
            //std::unordered_map <std::pair<const Stop*, const Stop*>, int, Hash> distance_stops;
            
            //проверить наличие остановок, если что запихнуть в очередь
            auto map_dist = it->second.AsMap();
            for (const auto& [name_d, dist] : map_dist){
                    if (catalog.FindStop(name_d) != nullptr) {
                        catalog.InputDistance(catalog.FindStop(stop.name), catalog.FindStop(name_d), dist.AsInt());
                    }
                    else {
                        
                        //waiting_stop_distance.insert ({stop.name, {name_d, dist.AsInt()}});
                        waiting_stop_distance[stop.name].insert ({name_d, dist.AsInt()});
                    }
            }
        }

}

void BusesCheck (transcat::TransportCatalogue& catalog, const Node& element_map){
    transcat::TransportCatalogue::Bus bus;
    auto it = element_map.AsMap().find("name"s);
    if (it != element_map.AsMap().end()) {
        bus.name = it->second.AsString();
    }
     
     it = element_map.AsMap().find("is_roundtrip"s);
    if (it != element_map.AsMap().end()) {
         bus.ring = it->second.AsBool();
    }

    it = element_map.AsMap().find("stops"s);
    if (it != element_map.AsMap().end()) {
        auto array = it->second.AsArray();
        if (!bus.ring) {
            std::deque<Node> temp {array.rbegin(), array.rend()};
            temp.pop_front();

            for (auto& stop : array) {
                bus.bus_stops.push_back(catalog.FindStop(stop.AsString()));
            }
            for (auto& stop : temp) {
                bus.bus_stops.push_back(catalog.FindStop(stop.AsString()));
            }
        }
        else {
            for (auto& stop : array) {
                bus.bus_stops.push_back(catalog.FindStop(stop.AsString()));
            }
        }
    }
    catalog.AddBusRoute(bus);
}


void FindStopsBuses (transcat::TransportCatalogue& catalog, std::deque <Document> documents) {
    std::unordered_map <std::string, std::set<std::pair <std::string, int>>> waiting_stop_distance;
    for (auto& doc: documents) {
        const json::Node* temp1 = &doc.GetRoot();
        bool test_clear_buses = false;
        if (!temp1->IsMap()) {
            
            std::cout << "Find error \n";
            auto test_array = temp1->AsArray();
            for ( auto& elem : test_array) {
                auto test2 = elem.AsMap();
                auto bus_clear = test2.find("buses"s);
                if ( bus_clear != test2.end()) {
                    test_clear_buses = true; // пропусть дальнейшее
                }
            }
            
        } 
        if (test_clear_buses)  {continue;}
        auto map_upper_level = (temp1->AsMap());
        auto it_map_ul = (map_upper_level).find("base_requests"s);
        if (it_map_ul == (map_upper_level).end()) { return;}
        auto map_base_requests = (it_map_ul->second).AsArray();
        
        
        for (const auto& element_map  : map_base_requests)  {
             auto it = element_map.AsMap().find("type"s);
            if (it != element_map.AsMap().end()) {
                std::string stop_check = it->second.AsString();
                if (stop_check == "Stop"s){
                   // std::cout << "Stop found"<< std::endl;
                    StopsCheck (catalog, waiting_stop_distance, element_map);
                }
            }
        }
        int all_stop_add = 0;
        do  {
                all_stop_add = 0;
                for (auto& stop_w : waiting_stop_distance) {
                    for (auto& [stop1, stop2_d] : stop_w.second) {
                        auto stop1_uk = catalog.FindStop(stop_w.first);
                        auto stop2_uk = catalog.FindStop(stop1);
                        if (stop1_uk != nullptr && stop2_uk != nullptr) {
                            catalog.InputDistance(stop1_uk, stop2_uk, stop2_d);
                        }
                        else {++all_stop_add;}
                    }
                }
        } while (all_stop_add != 0);

        for (const auto& element_map  : map_base_requests)  {
             auto it = element_map.AsMap().find("type"s);
            if (it != element_map.AsMap().end()) {
                std::string stop_check = it->second.AsString();
                if (stop_check == "Bus"s) {
                    //std::cout << "Bus found"<< std::endl;
                    BusesCheck (catalog, element_map);    
                }
            }
        }
    }
}

std::deque <request::RequestHandler::Request> ReadRequest ( std::deque <Document> documents) {
    std::deque <request::RequestHandler::Request> result;
    for (auto& doc: documents) {
        const json::Node* temp1 = &doc.GetRoot();
        if (!temp1->IsMap()) {
            std::cout << "не валидный запрос\n";
        }
        auto map_upper_level = (temp1->AsMap());
        auto it_map_ul = (map_upper_level).find("stat_requests"s);
        if (it_map_ul == (map_upper_level).end()) { return {};}
        auto array_base_requests = (it_map_ul->second).AsArray();
        for (const auto& map_req  : array_base_requests)  {
           request::RequestHandler::Request req_el;
           auto it = map_req.AsMap().find("id");
           if (it != map_req.AsMap().end()) {
                req_el.id  = it->second.AsInt();
                
           }
            it = map_req.AsMap().find("type");
           if (it != map_req.AsMap().end()) {
                req_el.type = it->second.AsString();
           }
            it = map_req.AsMap().find("name");
            if (it != map_req.AsMap().end()) {
                req_el.name = it->second.AsString();
           }
           result.push_back(req_el);
        }

     
    }
    return result;
}

std::string MakeRGB (int red, int green, int blue) {
        std::stringstream sstm;
        sstm << "rgb("sv << static_cast <int> (red) << ","sv << static_cast <int> (green) << ","sv<< static_cast <int> (blue) << ")"sv;
         return {sstm.str()};
}

std::string MakeRGBA (int red, int green, int blue, double opacity) {
        std::stringstream sstm;
        sstm << "rgba("sv << static_cast <int> (red) << ","sv 
             << static_cast <int> (green) << ","sv<< static_cast <int> (blue) 
             << ","sv << static_cast <double> (opacity)  << ")"sv;
        return {sstm.str()};
}

std::string MakeColor (json::Node color) {
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


Render_settings ReadRenderSettings (const std::deque <Document>& raw_documents) {
    Render_settings render_settings;
    for (auto& doc: raw_documents) {
        const json::Node* temp1 = &doc.GetRoot();
        if (!temp1->IsMap()) {
            std::cout << "не валидный запрос\n";
        }
        auto map_upper_level = (temp1->AsMap());
        auto it_map_ul = (map_upper_level).find("render_settings"s);
        if (it_map_ul == (map_upper_level).end()) { return {};}
        auto map_base_render = (it_map_ul->second).AsMap();
        for (const auto& [name, second] : map_base_render)  {
            if      (name == "width"s) { render_settings.width = second.AsDouble(); }
            else if (name == "height"s) { render_settings.height = second.AsDouble(); }
            else if (name == "padding"s) { render_settings.padding = second.AsDouble(); }
            else if (name == "line_width"s) { render_settings.line_width = second.AsDouble(); }
            else if (name == "stop_radius"s) { render_settings.stop_radius = second.AsDouble(); }
            else if (name == "bus_label_font_size"s) { render_settings.bus_label_font_size = second.AsInt(); }
            else if (name == "bus_label_offset"s) {
               render_settings.bus_label_offset.one = second.AsArray().at(0).AsDouble();
               render_settings.bus_label_offset.two = second.AsArray().at(1).AsDouble();
            }
            else if (name == "stop_label_font_size"s) { render_settings.stop_label_font_size = second.AsInt(); }
            else if (name == "stop_label_offset"s) {
               render_settings.stop_label_offset.one = second.AsArray().at(0).AsDouble();
               render_settings.stop_label_offset.two = second.AsArray().at(1).AsDouble();     
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
    return render_settings;
}


std::pair <std::deque <request::RequestHandler::Request>, Render_settings> json::read::LoadJSON (transcat::TransportCatalogue& cat, std::istream& input) {
    //Render_settings render_settings;
    std::deque <Document> raw_documents;
    
    while (input.peek() != EOF) {
        
        Document temp_doc = json::Load(input);
        if (!temp_doc.empty()) {raw_documents.push_back (temp_doc);}
        
    }
    
    FindStopsBuses (cat, raw_documents);
    Render_settings render_settings = ReadRenderSettings(raw_documents);
    auto result = ReadRequest ( raw_documents);
    return {result, render_settings};
    // return {};
}

Dict json::read::FormatANswerToJson (transcat::TransportCatalogue& cat, 
                                    request::RequestHandler::Answer& answer, RequestHandler& face) {
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
            auto date = cat.GetBusInfo (answer.name);
            //bus_name, count, uniq, real_distance, curv
            result.insert ( {"stop_count"s, Node ( static_cast<int>(std::get<1>(date)))} );
            result.insert ( {"unique_stop_count"s, Node ( static_cast<int>(std::get<2>(date)))} );
            result.insert ( {"route_length"s, Node ( std::get<3>(date))} );
            result.insert ( {"curvature"s, Node ( std::get<4>(date))} );
        }
    }
    if (answer.type == "Map"s) {
        auto docum = face.RenderMap();
        std::stringstream map_output;
        docum.Render(map_output);
        
        result.insert ( { "map"s, Node {map_output.str()} } );
    }
    return result;
}