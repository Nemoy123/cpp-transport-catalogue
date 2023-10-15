#include "request_handler.h"
#include <iostream>
#include "json_reader.h"

using namespace std::literals;
using namespace renderer;
using namespace svg;
using namespace transcat;

namespace request {

void RequestHandler::RequestRun () {
    
    for (auto& it : req_deq_) {
        Answer answer;
        answer.id = it.id;
        answer.type = it.type;
        
        if ((it).type == "Stop"s) {
            answer.name = it.name;
            //std::cout << "Стоп запрос" << std::endl;
            auto map_temp = db_.GetStopInfo((it).name); // ключ 0 имя стринг ключ 1 "not found" или "no buses"s, ключ 2 и далее стринг bus
            //std::string test1 = map_temp[1];
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
        // else if ((it).type == "Map"s) {
            
        // }
        answer_deq_.push_back(std::move(answer));
    }
}   


void RequestHandler::OutputRun() {
    Array result;
for (auto& ans : answer_deq_) {
    auto res = json::read::FormatANswerToJson (db_, ans, *this);
    result.push_back(Node (res));
   
}   

     json::PrintNode (result, output_);
}

class TextRoute : public Drawable {
public:
    TextRoute (const Render_settings& render_settings, renderer::SphereProjector& sphereprojector, 
            const transcat::TransportCatalogue::Bus* bus, int& index) : 
                render_settings_(render_settings),
                sphereprojector_(sphereprojector),
                bus_(bus), 
                index_(index)
    {
         for (auto stop: bus_->bus_stops) {
            const auto poin = sphereprojector_(stop->xy);
            stop_points_.push_back( poin );                 
        }
    }

    void Draw(svg::ObjectContainer& container) const override {
        svg::Text text_route;
            svg::Text text_2;
            
            text_route.SetPosition(stop_points_.front())
                      .SetOffset(svg::Point (render_settings_.bus_label_offset.one, render_settings_.bus_label_offset.two))
                      .SetFontSize(render_settings_.bus_label_font_size)
                      .SetFontFamily("Verdana"s)
                      .SetFontWeight("bold"s)
                      .SetData(bus_->name)
                      .SetFillColor(render_settings_.color_palette[index_])
                      ;

                text_2.SetPosition(stop_points_.front())
                      .SetOffset(svg::Point (render_settings_.bus_label_offset.one, render_settings_.bus_label_offset.two))
                      .SetFontSize(render_settings_.bus_label_font_size)
                      .SetFontFamily("Verdana"s)
                      .SetFontWeight("bold"s)
                      .SetData(bus_->name)
                      .SetFillColor(render_settings_.underlayer_color)
                      .SetStrokeColor(render_settings_.underlayer_color)
                      .SetStrokeWidth(render_settings_.underlayer_width)
                      .SetStrokeLineCap(StrokeLineCap::ROUND)
                      .SetStrokeLineJoin(StrokeLineJoin::ROUND)
                      ;
        container.Add (text_2);
        container.Add (text_route);
        

        if (!bus_->ring) {
            //убрать повторяющиеся
            std::deque <svg::Point> stop_points_noring;
           
            for (size_t i = 0; i < ((stop_points_.size()+1) / 2); ++i) {
                
                  stop_points_noring.push_back(*(stop_points_.begin() + i));
            }

            
            if (!(stop_points_.front() == stop_points_noring.back())) {
                text_route.SetPosition(stop_points_noring.back());
                text_2.SetPosition(stop_points_noring.back());

                container.Add (text_2);
                container.Add (text_route);
            }
            
        }
    }

private:
   
    const Render_settings& render_settings_;
    renderer::SphereProjector& sphereprojector_;
    const transcat::TransportCatalogue::Bus* bus_;
    int index_ = 0;
    std::deque <svg::Point> stop_points_;
    
};

class BusRoute : public Drawable {
public:
    
    BusRoute(const Render_settings& render_settings , renderer::SphereProjector& sphereprojector, 
            const transcat::TransportCatalogue::Bus* bus, int& index) : 
                render_settings_(render_settings),
                sphereprojector_(sphereprojector),
                bus_(bus), 
                index_(index)
    {
         for (auto stop: bus_->bus_stops) {
            const auto poin = sphereprojector_(stop->xy);
            // std::deque <svg::Point> stop_points_;
            stop_points_.push_back( poin );                 
        }
    }

    // Реализует метод Draw интерфейса svg::Drawable
    void Draw(svg::ObjectContainer& container) const override {

        svg::Polyline it;
        for (const auto& point : stop_points_) {
             it.AddPoint(point);
        }
        it.SetStrokeWidth(render_settings_.line_width);
        it.SetStrokeColor(render_settings_.color_palette[index_]);
        it.SetFillColor("none"s);
        it.SetStrokeLineCap(StrokeLineCap::ROUND);
        it.SetStrokeLineJoin(StrokeLineJoin::ROUND);

        container.Add (it);

    }

private:
     
    const Render_settings& render_settings_;
    renderer::SphereProjector& sphereprojector_;
    const transcat::TransportCatalogue::Bus* bus_;
    int index_ = 0;
    std::deque <svg::Point> stop_points_;
   
};

class CircleStop : public Drawable {
public:
    CircleStop(const Render_settings& render_settings , svg::Point circle) : 
            render_settings_(render_settings), 
            circle_(circle)
             {}

    void Draw(svg::ObjectContainer& container) const override {
        svg::Circle stop;
    
        stop.SetRadius(render_settings_.stop_radius);
        stop.SetFillColor("white");
        stop.SetCenter(circle_);
        container.Add (stop);
    }
private:
    
    const Render_settings& render_settings_;
    svg::Point circle_;
};

class StopText : public Drawable {
public:
    StopText(const Render_settings& render_settings , svg::Point circle, std::string_view name) : 
            render_settings_(render_settings), 
            circle_(circle),
            name_(std::string{name})
             {}

    void Draw(svg::ObjectContainer& container) const override {
        svg::Text text_main;
        svg::Text text_s;

             text_main.SetPosition(circle_)
                      .SetOffset(svg::Point (render_settings_.stop_label_offset.one, render_settings_.stop_label_offset.two))
                      .SetFontSize(render_settings_.stop_label_font_size)
                      .SetFontFamily("Verdana"s)
                      .SetData(name_)
                      .SetFillColor("black")  
                      ;
                text_s.SetPosition(circle_)
                      .SetOffset(svg::Point (render_settings_.stop_label_offset.one, render_settings_.stop_label_offset.two))
                      .SetFontSize(render_settings_.stop_label_font_size)
                      .SetFontFamily("Verdana"s)
                      .SetData(name_)
                      .SetFillColor(render_settings_.underlayer_color)
                      .SetStrokeColor(render_settings_.underlayer_color)
                      .SetStrokeWidth(render_settings_.underlayer_width)
                      .SetStrokeLineCap(StrokeLineCap::ROUND)
                      .SetStrokeLineJoin(StrokeLineJoin::ROUND)
                      ;
                      container.Add (text_s);
                      container.Add (text_main);

    }
private:
    const Render_settings& render_settings_;
    svg::Point circle_;
    std::string name_;
    
    

};

template <typename DrawableIterator>
void DrawPicture(DrawableIterator begin, DrawableIterator end, svg::ObjectContainer& target) {
    for (auto it = begin; it != end; ++it) {
        (*it)->Draw(target);
    }
}

template <typename Container>
void DrawPicture(const Container& container, svg::ObjectContainer& target) {
    using namespace std;
    DrawPicture(begin(container), end(container), target);
}


svg::Document RequestHandler::RenderMap() const {
    svg::Document result;
    std::deque <std::unique_ptr<svg::Drawable>> picture;
    
    std::deque <geo::Coordinates> coor_deq;
    for (const auto [name, bus] : db_.GetRoutes()){
        
        for (auto stop : bus->bus_stops) {
            coor_deq.push_back(stop->xy);
        }
    }
    renderer::SphereProjector sphereprojector (std::cbegin(coor_deq), std::cend(coor_deq), 
                                    render_settings_.width, render_settings_.height, render_settings_.padding );
        int i = 0;
      std::map <std::string_view, const transcat::TransportCatalogue::Bus*> map_routes;
      for (const auto [name, bus] : db_.GetRoutes()) {
            map_routes.emplace(name, bus);
      }
      
      for (const auto [name, bus] : map_routes){
           
            picture.emplace_back( std::make_unique<BusRoute>(render_settings_, sphereprojector, bus, i) );
            ++i;
       }
       i = 0;
       for (const auto [name, bus] : map_routes){
           
            picture.emplace_back( std::make_unique<TextRoute>(render_settings_, sphereprojector, bus, i) );
            ++i;
       }
        
        std::map <std::string_view, svg::Point> map_circle;
        for (const auto [name, bus] : map_routes){
           
            for (auto& stop_dr : bus->bus_stops) {
                map_circle.emplace(stop_dr->name, sphereprojector(stop_dr->xy));
            }
        }
        for (auto& obj : map_circle){
                picture.emplace_back ( std::make_unique<CircleStop>(render_settings_, obj.second) );
        }
        for (auto& obj : map_circle){
                picture.emplace_back ( std::make_unique<StopText>(render_settings_, obj.second, obj.first) );
        }

       DrawPicture(picture, result);
    
    return result;
}

void LoadInput (std::istream& input_t, std::ostream& output) {
    transcat::TransportCatalogue cat;
    request::RequestHandler face (cat, input_t, output);
   auto temp =  json::read::LoadJSON (cat, input_t);
   face.req_deq_ = temp.first;
   face.render_settings_ = temp.second;
   auto docum = face.RenderMap();
   face.RequestRun ();
   //docum.Render(output);
   
    face.OutputRun();
}  

} // end namespace 