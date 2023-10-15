/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

#pragma once
#include "transport_catalogue.h"
#include "request_handler.h"
#include "json.h"
#include "map_renderer.h"

using namespace json;
using  namespace request;
using namespace renderer;

namespace json::read {


std::pair <std::deque <request::RequestHandler::Request>, Render_settings> LoadJSON (transcat::TransportCatalogue& cat, std::istream& input);
Dict FormatANswerToJson (transcat::TransportCatalogue& cat, request::RequestHandler::Answer& answer, request::RequestHandler& face);
        
 

    
}



