#pragma once
#include <optional>
#include <set>
#include "map_renderer.h"
#include "domain.h"
#include "transport_catalogue.h"

#include "router.h"
#include "transport_router.h"
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
        std::string from_stop;
        std::string to_stop;
    };

    struct InfoEdge {
        graph::Edge<double> edge;
        std::string_view bus_name;
        //std::vector <graph::Edge<double>> previous_segments_in_edge;
        size_t span_count = 0;
    };

    struct Answer {
        int id;
        std::string name;
        std::string type;
        std::set <std::string> ans_set;
        bool not_found_stop = false;
        bool not_found_buses = false;
        bool not_found_route = false;
        graph::Router<double>::RouteInfo route_date;
        //std::vector<InfoEdge> edge_info;
        //std::optional<graph::Router<int>::RouteInfo> route_info;
    };

    
    
    RequestHandler (transcat::TransportCatalogue& cat, std::istream& input, std::ostream& output) : 
                        db_(cat), 
                        input_(input), 
                        output_(output)
                        // dw_graph_(),
                        //router_(dw_graph_)
    {
                         
                         //graph::DirectedWeightedGraph<double> dw_graph_ (db_.GetAllStops().size());
    }
    // MapRenderer понадобится в следующей части итогового проекта
    //RequestHandler(const transcat::TransportCatalogue& db, const renderer::MapRenderer& renderer);

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;

    // Возвращает маршруты, проходящие через
    std::unordered_set<const Bus*> GetBusesByStop(const std::string_view& stop_name) const;

    // Этот метод будет нужен в следующей части итогового проекта
    svg::Document RenderMap() const;
    
    void ExecuteRequests ();
    //void OutputRun();
    const transcat::TransportCatalogue& GetTransportBase () const { return db_; }
    std::deque <Answer>& GetAnswerDeq () {return answer_deq_;}
    std::ostream& GetOutput () {return output_;}
    void InputRequestDeque (std::deque <Request> inp) {req_deq_ = std::move(inp);}
    void InputRenderSettings (RenderSettings rs) {render_settings_ = std::move(rs);}
    const RenderSettings& GetRenderSettings() {return render_settings_;}
    // const graph::DirectedWeightedGraph<double>& GetGraph () const {
    //     auto temp = router_.GetGraph();
    //     return temp;}
    TransportRouter<double>& GetRouter () {return router_;}
    

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    transcat::TransportCatalogue& db_;
    
    std::istream& input_;
    std::ostream& output_;
    //const renderer::MapRenderer& renderer_;
    std::deque <Answer> answer_deq_ = {}; // очередь ответа
    //json::read::JSONReader json_reader_ = {db_, this, input_};
    std::deque <Request> req_deq_ = {}; // очередь выполнения
    RenderSettings render_settings_;
    //graph::DirectedWeightedGraph<double> dw_graph_;
    //graph::Router<double> router_;
    
    //void YouCanMakeItRealRouter (); //const Request& it
    TransportRouter<double> router_;

};




void LoadInput (std::istream& input, std::ostream& output);

} // конец namespace