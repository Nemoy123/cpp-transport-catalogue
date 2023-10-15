#pragma once
#include <optional>
#include <set>
#include "map_renderer.h"
#include "domain.h"
#include "transport_catalogue.h"
//

using BusStat = std::tuple <std::string_view, std::size_t, std::size_t, double, double>;

//
using namespace renderer;

namespace request {


class RequestHandler {
public:
    
    struct Request {
        int id;
        std::string type;
        std::string name;
    };

    struct Answer {
        int id;
        std::string name;
        std::string type;
        std::set <std::string> ans_set;
        bool not_found_stop = false;
        bool not_found_buses = false;
    };

    std::deque <Request> req_deq_ = {}; // очередь выполнения
    
    RequestHandler (transcat::TransportCatalogue& cat, std::istream& input, std::ostream& output) : 
                        db_(cat), 
                        input_(input), 
                        output_(output)
                        {}
    // MapRenderer понадобится в следующей части итогового проекта
    //RequestHandler(const transcat::TransportCatalogue& db, const renderer::MapRenderer& renderer);

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;

    // Возвращает маршруты, проходящие через
    std::unordered_set<const Bus*> GetBusesByStop(const std::string_view& stop_name) const;

    // Этот метод будет нужен в следующей части итогового проекта
    svg::Document RenderMap() const;
    
    void RequestRun ();
    void OutputRun();
    RenderSettings render_settings_;
    const transcat::TransportCatalogue& GetTransportBase () const { return db_; }
    //void OutputRun(json::read::JSONReader& json_reeder); 
    void MakeJSONReader ();
    std::deque <Answer>& GetAnswerDeq () {return answer_deq_;}
    std::ostream& GetOutput () {return output_;}
    //void OutputRun(T& json_reeder);

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    transcat::TransportCatalogue& db_;
    
    std::istream& input_;
    std::ostream& output_;
    //const renderer::MapRenderer& renderer_;
    std::deque <Answer> answer_deq_ = {}; // очередь ответа
    //json::read::JSONReader json_reeder_ = {db_, this, input_};

    

};




void LoadInput (std::istream& input, std::ostream& output);

} // конец namespace