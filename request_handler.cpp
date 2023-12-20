#include "request_handler.h"
#include <iostream>
#include <algorithm>
//#include <execution>
#include <mutex>
#include <unordered_set>
#include "json_reader.h"
// #include "log_duration.h"
#include "router.h"
#include "serialization.h"

using namespace std::literals;
using namespace renderer;
using namespace svg;
using namespace transcat;
using namespace json;


namespace request {



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
                
                    auto iter1 = std::find ( (db_.GetAllStops()).cbegin(),(db_.GetAllStops()).cend(), *(db_.FindStop(it.from_stop))); // 
                    size_t from_stop_size = iter1 - (db_.GetAllStops()).cbegin();
                    auto iter2 = std::find ( (db_.GetAllStops()).cbegin(), (db_.GetAllStops()).cend(), *(db_.FindStop(it.to_stop))); //
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
    if (answer_deq_.empty()) {std::cout << "answer_deq_ empty!!!" << std::endl;}
    json::Array result;
    for (request::RequestHandler::Answer& ans : answer_deq_) {
        // std::cout << "void OutputRun 3" << std::endl;
        auto res = json_reader.FormatAnsertToJSONBuilder (ans);
        // std::cout << "void OutputRun 4" << std::endl;
        result.push_back(Node (res));
    }   
    // std::cout << "void OutputRun 1" << std::endl;
     json::PrintNode (result, output_);
    //  std::cout << "void OutputRun 2" << std::endl;
}


svg::Document RequestHandler::RenderMap() const {
    MapRender map_render (render_settings_, db_);
    return map_render.MapRenderer();
}

void MakeBase (std::istream& input) {
    transcat::TransportCatalogue cat;
    request::RequestHandler face (cat, input, std::cout);
    json::read::JSONReader json_reader (cat, face, input);
    json_reader.LoadJSON();
    face.InputRenderSettings(json_reader.GiveRenderSettings());

    //std::cout << "json_reader.GiveSerializationSettings() " << json_reader.GiveSerializationSettings() << std::endl;

    face.InputSerializationSettings(json_reader.GiveSerializationSettings());

    //std::cout << "face.GetSerializationSettings() " << face.GetSerializationSettings() << std::endl;

    SavingDB saving_db (face.GetSerializationSettings());
    if (!saving_db.SerializeDB(cat, face.GetRenderSettings())) {
        std::cout << "Error SerializeDB" << std::endl;
    }
}

void ProcessRequests (std::istream& input, std::ostream& output) {
    transcat::TransportCatalogue cat;
    request::RequestHandler face (cat, input, output);
    json::read::JSONReader json_reader (cat, face, input);
    json_reader.LoadJSONReadSerializationSettings ();
    SavingDB saving_db (json_reader.GiveSerializationSettings());
    if (!saving_db.DeserializeDB(cat, face)) { 
        std::cout << "Error DeserializeDB" << std::endl;
    }
    json_reader.LoadJSONFromSavedInput();

    //std::cout << "json_reader.LoadJSONFromSavedInput() done"<< std::endl;
    face.InputRequestDeque(json_reader.GiveRequests());
    //std::cout << "face.InputRequestDeque(json_reader.GiveRequests()); done"<< std::endl;
    MapRender map_render(face.GetRenderSettings(), face.GetTransportBase());
    //std::cout << "MapRender map_render(face.GetRenderSettings(), face.GetTransportBase()); done"<< std::endl;
    face.ExecuteRequests ();
    // std::cout << "face.ExecuteRequests (); done"<< std::endl;
    OutputRun(json_reader, face.GetAnswerDeq(), face.GetOutput());
    // std::cout << "OutputRun done"<< std::endl;
}

// void LoadInput (std::istream& input_t, std::ostream& output) {
//     transcat::TransportCatalogue cat;
//     request::RequestHandler face (cat, input_t, output);
//     json::read::JSONReader json_reader (cat, face, input_t);
//     // {
//     //     LogDuration load ("load time "s);
//         json_reader.LoadJSON();
//     // }
//     // {
//     //     LogDuration input_request_deque ("input_request_deque time "s);
//         face.InputRequestDeque(json_reader.GiveRequests());
//     // }
//     // {
//     //     LogDuration input_render ("input_input_render time "s);
//         face.InputRenderSettings(json_reader.GiveRenderSettings());
//     // }
//     // {
//     //     LogDuration time_map_render ("input_map_render time "s);
//         face.InputSerializationSettings(json_reader.GiveSerializationSettings());

//         MapRender map_render(face.GetRenderSettings(), face.GetTransportBase());
//     //}
    

//     // { 
//     //     LogDuration exec_requests ("exec_requests time "s);
//         face.ExecuteRequests ();
//     // }

//     // { 
//     //     LogDuration output_run ("output_run time "s); 
//         OutputRun(json_reader, face.GetAnswerDeq(), face.GetOutput());
//     // }
// }  

} // end namespace 