#include "request_handler.h"
#include <iostream>
#include "json_reader.h"

using namespace std::literals;
using namespace renderer;
using namespace svg;
using namespace transcat;
using namespace json;

namespace request {

void RequestHandler::ExecuteRequests () {
    
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
        
        answer_deq_.push_back(std::move(answer));
    }
}   


template <typename T>
void OutputRun( T& json_reader, std::deque <request::RequestHandler::Answer>& answer_deq_, std::ostream& output_) {
    
    json::Array result;
    for (request::RequestHandler::Answer& ans : answer_deq_) {
        json::Dict res = json_reader.FormatAnsertToJSON (ans);
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
    json_reader.LoadJSON();

   //auto temp =  json::read::LoadJSON (cat, input_t);

    face.InputRequestDeque(json_reader.GiveRequests());
// //    face.req_deq_ = temp.first;
// //    face.render_settings_ = temp.second;
    face.InputRenderSettings(json_reader.GiveRenderSettings());
// //    auto docum = face.RenderMap();
    MapRender map_render(face.GetRenderSettings(), face.GetTransportBase());
//     auto docum = map_render.MapRenderer ();
     face.ExecuteRequests ();
     OutputRun(json_reader, face.GetAnswerDeq(), face.GetOutput());
}  

} // end namespace 