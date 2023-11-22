#pragma once
#include <string>
#include <deque>
#include "geo.h"

/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области (domain)
 * вашего приложения и не зависят от транспортного справочника. Например Автобусные маршруты и Остановки. 
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */

struct Stop {
        std::string name;
        geo::Coordinates xy;
        bool operator== (const Stop& other) const {return name == other.name && xy == other.xy;} 
};

struct Bus {
        std::string name;
        std::deque <const Stop*> bus_stops;
        bool ring;
};

struct BusInfo {
    std::string_view name;
    std::size_t count_stops;
    std::size_t uniq_stops;
    double real_dist;
    double curv;
};