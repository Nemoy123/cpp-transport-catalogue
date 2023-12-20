#include "serialization.h"
#include "transport_catalogue.pb.h"
#include <fstream>


bool SavingDB::SerializeDB (const transcat::TransportCatalogue& cat, const renderer::RenderSettings& ren_set) const {
    

    transport_proto::TransportCatalogue catalog;
    for (const auto& stop : cat.GetAllStops()) {
        transport_proto::Stop stop_proto;
        transport_proto::Coordinates coor;
        coor.set_lat(stop.xy.lat);
        coor.set_lng(stop.xy.lng);
        stop_proto.set_name(stop.name);
        *stop_proto.mutable_xy() = coor;
        *catalog.add_all_stops() = stop_proto;
    }
    for (const auto& bus : cat.GetAllBuses()) {
        transport_proto::Bus bus_proto;
        *bus_proto.mutable_name() = bus.name;
         bus_proto.set_ring(bus.ring);
         for (const auto& stop :bus.bus_stops) {
            *bus_proto.add_stops() = stop->name;
         }
         *catalog.add_all_buses() = bus_proto;
    }
    for (const auto& [pair_stops, dist] : cat.GetAllDistances()) {
        transport_proto::StopsDistance std;
        *std.mutable_stop1() = pair_stops.first->name;
        *std.mutable_stop2() = pair_stops.second->name;
        std.set_distance(dist);
        *catalog.add_stop_distance() = std;
    }
    catalog.mutable_routing_settings()->set_bus_velocity(cat.GetRoutingSet().bus_velocity);
    catalog.mutable_routing_settings()->set_bus_wait_time(cat.GetRoutingSet().bus_wait_time);

    catalog.mutable_render_settings()->set_width(ren_set.width);
    catalog.mutable_render_settings()->set_height(ren_set.height);
    catalog.mutable_render_settings()->set_padding(ren_set.padding);
    catalog.mutable_render_settings()->set_line_width(ren_set.line_width);
    catalog.mutable_render_settings()->set_stop_radius(ren_set.stop_radius);
    catalog.mutable_render_settings()->set_bus_label_font_size(ren_set.bus_label_font_size);
    catalog.mutable_render_settings()->mutable_bus_label_offset()->set_dx(ren_set.bus_label_offset.dx);
    catalog.mutable_render_settings()->mutable_bus_label_offset()->set_dy(ren_set.bus_label_offset.dy);
    catalog.mutable_render_settings()->set_stop_label_font_size(ren_set.stop_label_font_size);
    catalog.mutable_render_settings()->mutable_stop_label_offset()->set_dx(ren_set.stop_label_offset.dx);
    catalog.mutable_render_settings()->mutable_stop_label_offset()->set_dy(ren_set.stop_label_offset.dy);
    catalog.mutable_render_settings()->set_underlayer_color(ren_set.underlayer_color);
    catalog.mutable_render_settings()->set_underlayer_width(ren_set.underlayer_width);
    for (auto i = 0; i < ren_set.color_palette.color_palette.size(); ++i) {
        catalog.mutable_render_settings()->mutable_color_palette()->add_color(ren_set.color_palette.color_palette.at(i));
    }

    std::ofstream out(file_name_, std::ios::binary);
    // std::cout << "file_name_ " << file_name_ << std::endl;
    if (catalog.SerializeToOstream(&out)) {
        return true;
    }
    else {return false;}
    
}

bool SavingDB::DeserializeDB (transcat::TransportCatalogue& cat, request::RequestHandler& face) {
    std::ifstream in (file_name_, std::ios::binary);
    transport_proto::TransportCatalogue catalog;
    catalog.ParseFromIstream(&in);

    transcat::TransportCatalogue::RoutingSet set;
    set.bus_velocity = catalog.mutable_routing_settings()->bus_velocity();
    set.bus_wait_time = catalog.mutable_routing_settings()->bus_wait_time();
    cat.SetRoutingSet(std::move(set));

    auto size_catalog_all_stops_size = catalog.all_stops_size();
    for (auto i = 0; i < size_catalog_all_stops_size; ++i) {
        Stop stop;
        stop.name = catalog.all_stops(i).name();
        stop.xy.lat = catalog.all_stops(i).xy().lat();
        stop.xy.lng = catalog.all_stops(i).xy().lng();
        cat.AddStop(std::move(stop));
    }
    auto size_catalog_all_buses_size = catalog.all_buses_size();
    for (auto i = 0; i < catalog.all_buses_size(); ++i) { 
        Bus bus;
        bus.name = catalog.all_buses(i).name();
        bus.ring = catalog.all_buses(i).ring();
        std::deque<const Stop *> stops_deq;
        for (auto t = 0; t < catalog.all_buses(i).stops_size(); ++t) {
           auto ptr_stop = cat.FindStop(catalog.all_buses(i).stops(t));
           stops_deq.push_back(ptr_stop);
        }
        bus.bus_stops = std::move(stops_deq);
        cat.AddBusRoute(std::move(bus));
    }
    auto size_catalog_stop_distance_size = catalog.stop_distance_size();
    for (auto i = 0; i < catalog.stop_distance_size(); ++i) {  
        std::pair<const Stop*, const Stop*> pair_st;
        int dist = catalog.stop_distance(i).distance();
        pair_st.first = cat.FindStop(catalog.stop_distance(i).stop1());
        pair_st.second = cat.FindStop(catalog.stop_distance(i).stop2());
        cat.InputDistance(pair_st.first, pair_st.second, dist);
    }
    RenderSettings rs;
    rs.width = catalog.mutable_render_settings()->width();
    rs.height = catalog.mutable_render_settings()->height();
    rs.padding = catalog.mutable_render_settings()->padding();
    rs.line_width = catalog.mutable_render_settings()->line_width();
    rs.stop_radius = catalog.mutable_render_settings()->stop_radius();
    rs.bus_label_font_size = catalog.mutable_render_settings()->bus_label_font_size();
    rs.bus_label_offset.dx = catalog.mutable_render_settings()->bus_label_offset().dx();
    rs.bus_label_offset.dy = catalog.mutable_render_settings()->bus_label_offset().dy();
    rs.stop_label_font_size = catalog.mutable_render_settings()->stop_label_font_size();
    rs.stop_label_offset.dx = catalog.mutable_render_settings()->stop_label_offset().dx();
    rs.stop_label_offset.dy = catalog.mutable_render_settings()->stop_label_offset().dy();
    rs.underlayer_color = catalog.mutable_render_settings()->underlayer_color();
    rs.underlayer_width = catalog.mutable_render_settings()->underlayer_width();
    for (auto i = 0; i < catalog.mutable_render_settings()->color_palette().color_size(); ++i) {
        rs.color_palette.color_palette.push_back(catalog.mutable_render_settings()->color_palette().color(i));
    }
    face.InputRenderSettings(std::move(rs));
    return true;
}