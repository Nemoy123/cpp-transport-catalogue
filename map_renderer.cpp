#include "map_renderer.h"
#include <algorithm>

using namespace std::literals;

using namespace svg;
using namespace transcat;

namespace renderer {

bool IsZeroTest(double value) {
    const double EPSILON = 1e-6;
            return std::abs(value) < EPSILON;
}

class TextRoute : public Drawable {
public:
    TextRoute (const RenderSettings& render_settings, renderer::SphereProjector& sphereprojector, 
            const Bus* bus, int& index) : 
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
                      .SetOffset(svg::Point (render_settings_.bus_label_offset.dx, render_settings_.bus_label_offset.dy))
                      .SetFontSize(render_settings_.bus_label_font_size)
                      .SetFontFamily("Verdana"s)
                      .SetFontWeight("bold"s)
                      .SetData(bus_->name)
                      .SetFillColor(render_settings_.color_palette[index_])
                      ;

                text_2.SetPosition(stop_points_.front())
                      .SetOffset(svg::Point (render_settings_.bus_label_offset.dx, render_settings_.bus_label_offset.dy))
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
   
    const RenderSettings& render_settings_;
    renderer::SphereProjector& sphereprojector_;
    const Bus* bus_;
    int index_ = 0;
    std::deque <svg::Point> stop_points_;
    
};


class BusRoute : public Drawable {
public:
    
    BusRoute(const RenderSettings& render_settings , renderer::SphereProjector& sphereprojector, 
            const Bus* bus, int& index) : 
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
     
    const RenderSettings& render_settings_;
    renderer::SphereProjector& sphereprojector_;
    const Bus* bus_;
    int index_ = 0;
    std::deque <svg::Point> stop_points_;
   
};

class CircleStop : public Drawable {
public:
    CircleStop(const RenderSettings& render_settings , svg::Point circle) : 
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
    
    const RenderSettings& render_settings_;
    svg::Point circle_;
};


class StopText : public Drawable {
public:
    StopText(const RenderSettings& render_settings , svg::Point circle, std::string_view name) : 
            render_settings_(render_settings), 
            circle_(circle),
            name_(std::string{name})
             {}

    void Draw(svg::ObjectContainer& container) const override {
        svg::Text text_main;
        svg::Text text_s;

             text_main.SetPosition(circle_)
                      .SetOffset(svg::Point (render_settings_.stop_label_offset.dx, render_settings_.stop_label_offset.dy))
                      .SetFontSize(render_settings_.stop_label_font_size)
                      .SetFontFamily("Verdana"s)
                      .SetData(name_)
                      .SetFillColor("black")  
                      ;
                text_s.SetPosition(circle_)
                      .SetOffset(svg::Point (render_settings_.stop_label_offset.dx, render_settings_.stop_label_offset.dy))
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
    const RenderSettings& render_settings_;
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

svg::Document MapRender::MapRenderer () const {
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
      std::map <std::string_view, const Bus*> map_routes;
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


} // namespace end