#pragma once
#include "transport_catalogue.h"
#include "request_handler.h"
#include "json.h"
#include "json_builder.h"
#include "map_renderer.h"

//using namespace json;
//using  namespace request;
using namespace renderer;

namespace json::read {

class JSONReader {

public:
    JSONReader (transcat::TransportCatalogue& cat, request::RequestHandler& face, std::istream& input) :
        cat_(cat),
        face_(face),
        input_(input)
     {} 
    // void FillStop ( transcat::TransportCatalogue& catalog, 
    //                   std::unordered_map <std::string, std::set<std::pair <std::string, int>>>& waiting_stop_distance, 
    //                   const Node& element_map );
    void FillStop ( std::unordered_map <std::string, std::set<std::pair <std::string, int>>>& waiting_stop_distance, 
                      const Node& element_map );
    //void FillBus (transcat::TransportCatalogue& catalog, const Node& element_map);
    void FillBus (const Node& element_map);

    //void FillCatalogue (transcat::TransportCatalogue& catalog, std::deque <Document> documents);
    void FillCatalogue (const std::deque <Document>& documents);
    std::deque <request::RequestHandler::Request> ReadRequest ( std::deque <Document>&& documents); 
    std::string MakeRGB (int red, int green, int blue);
    std::string MakeRGBA (int red, int green, int blue, double opacity);
    std::string MakeColor (json::Node color);
    void ReadRenderSettings (const std::deque <Document>& raw_documents);
    transcat::TransportCatalogue::RoutingSet ReadRoutingSettings (const std::deque <Document>& raw_documents);
    void LoadJSON (); //std::pair <std::deque <request::RequestHandler::Request>, RenderSettings>

    Dict FormatAnsertToJSON (request::RequestHandler::Answer& answer);
    Node FormatAnsertToJSONBuilder (request::RequestHandler::Answer& answer); 

    const std::deque <request::RequestHandler::Request>& GiveRequests () const { return requests_;}
    const RenderSettings& GiveRenderSettings () const {return render_settings_;}
    const std::string& GiveSerializationSettings() const {return ser_settings_;}
    void ReadSerializationSettings (const std::deque <Document>& raw_documents);
    void LoadJSONReadSerializationSettings ();
    std::deque <Document> input_raw_documents; // входящий запрос
    void LoadJSONFromSavedInput (); //ввод данных из сохраненных, а не из потока
private:
    transcat::TransportCatalogue& cat_;
    request::RequestHandler& face_;
    std::istream& input_;
    RenderSettings render_settings_;
    std::deque <request::RequestHandler::Request> requests_;
    std::string ser_settings_;

};

//std::pair <std::deque <request::RequestHandler::Request>, RenderSettings> LoadJSON (transcat::TransportCatalogue& cat, std::istream& input);
//Dict FormatAnsertToJSON (transcat::TransportCatalogue& cat, request::RequestHandler::Answer& answer, request::RequestHandler& face);
        
 

    
}



