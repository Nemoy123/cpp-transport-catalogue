#include <string>
#include "transport_catalogue.h"
#include "request_handler.h"


class SavingDB {


    public:
        SavingDB(const std::string& file_name) : file_name_(file_name) {

            //std::cout << "SavingDB file_name_ " << file_name_ << std::endl;
        }

        bool SerializeDB (const  transcat::TransportCatalogue& cat, const renderer::RenderSettings& ren_set) const;
        bool DeserializeDB (transcat::TransportCatalogue& cat, request::RequestHandler& face);

    private:
        const std::string file_name_;
};