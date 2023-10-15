#pragma once
#include "svg.h"
#include "transport_catalogue.h"
#include "geo.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>

using namespace transcat;
namespace renderer {

bool IsZeroTest(double value);


class SphereProjector {
public:
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                    double max_width, double max_height, double padding)
        : padding_(padding) //
    {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end) {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;
        
        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZeroTest(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZeroTest(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        } else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};


class ColorPalette {
public:
    std::deque <std::string> color_palette;
    
    const std::string& operator[](const int index) const {
        if (index < static_cast<int> (color_palette.size())) return color_palette.at(index);
        else  { 
            int size = static_cast<int> (color_palette.size());
            int temp =  index / size ; //  без дробной части
            int new_ind = (index - (size * temp));
            return color_palette.at(new_ind);
        }
    }
};


struct RenderSettings {
    double width = 0;
    double height = 0;
    double padding = 0;
    double line_width = 0;
    double stop_radius = 0;
    int bus_label_font_size = 0;
    
    struct BusLabelOffset {
        double dx = 0;
        double dy = 0;
    };
    BusLabelOffset bus_label_offset;
    int stop_label_font_size = 0;

    struct StopLabelOffset {
        double dx = 0;
        double dy = 0;
    };
    StopLabelOffset stop_label_offset;
    std::string underlayer_color;
    
    double underlayer_width = 0;
   
    ColorPalette color_palette;
};




class MapRender {
    public:
    MapRender (const RenderSettings& render_settings, const transcat::TransportCatalogue& db) :
        render_settings_(render_settings),
        db_(db)
    {}
    svg::Document MapRenderer () const;
    
    private:

    const RenderSettings& render_settings_;
    const transcat::TransportCatalogue& db_;


};

} // конец namespace