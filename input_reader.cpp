
#include <string>
//#include <execution>
#include "domain.h"
#include "input_reader.h"
#include "stat_reader.h"


using std::string;
using std::endl;
using std::move;
using std::string_view;

using namespace transcat::output;
namespace transcat::input {

using waiting_bus = std::deque <std::pair <Bus, std::deque<std::string>>>;
using dist_wait = std::deque <std::tuple <string, string, int>>;

string_view PureString (string_view& line) {
    auto pos_end = line.find_last_not_of(" ");
    line = line.substr(0, pos_end+1);
    auto pos = line.find_first_not_of(" ");
    line = line.substr(pos, line.size());
    return line;
}

std::pair <std::deque <string>, bool> SplitBusStop (string_view text_all) {
    if (text_all.empty()) return {};
    std::deque <string> result;
    string_view text = text_all;
    bool round =true;
    auto begin = 0;
    auto end = text.find_first_of ('>');
    if ( end == std::string::npos) {
        round = false;
        end = text.find_first_of ('-');
        if (end == std::string::npos) {
            return {};
        }
    }
    while (begin != std::string::npos) {
        string_view word = text.substr(begin, end-1);
        PureString (word);
        string word_str {word};
        result.push_back(word_str);
        if (end + 1 != std::string::npos) {text = text.substr(end+1);}
        else {break;}
        end = round ? text.find_first_of ('>') : text.find_first_of ('-');
        if ( end == std::string::npos) {
            string_view word = text.substr(begin);
            PureString (word);
            string word_str {word};
            result.push_back(word_str);
            break;
        }
    }
    return {result, round};
}

bool CheckStops (transcat::TransportCatalogue& cat, std::deque<string>& res_deq, Bus& out) {
    bool check = true;
    for (auto& str : res_deq) {
        auto stop = cat.FindStop (str);
        if ( stop == nullptr) {
            check = false;
            break;
        }
        (out.bus_stops).push_back( stop );
    }   
    if (!check) {
        out.bus_stops.clear();
    }
    return check;
}
void BusInput (std::ostream& output_cout, transcat::TransportCatalogue& cat, waiting_bus& buses_wait_add, 
               std::string& line) {
        Bus result;
        auto pos_end_name = line.find_first_of(':');
        std::string bus_name ={};
        if (pos_end_name != std::string::npos) {
            bus_name = line.substr(4, pos_end_name-4);
        } 
        else {
                bus_name = line.substr(4); 
        }
        auto res_deq { SplitBusStop (line.substr(pos_end_name+1)) };
        if (res_deq.first.empty()) {
            
           // auto bus_find = cat.FindBus(bus_name);
            auto out_res = cat.GetBusInfo (bus_name);
            transcat::output::DateOutput (output_cout, out_res);

        }
        else {

            result.name = bus_name;
            if (!res_deq.second) {
                std::deque<std::string> temp {res_deq.first.rbegin(), res_deq.first.rend()};
                temp.pop_front();
                for (auto& t : temp) {
                    res_deq.first.push_back(t);
                }
            }
            result.ring = res_deq.second;
            auto dfgd = CheckStops (cat, res_deq.first, result);
            if (dfgd) {
                cat.AddBusRoute (move(result));
            }
            else {
                buses_wait_add.push_back ({result, move(res_deq.first)});
            }
        }
}
void StopInput (std::ostream& output_cout, transcat::TransportCatalogue& cat, dist_wait& dist_waiting_add, std::string& line) {
        Stop result;
        line = line.substr(5);
        auto pos_end_name = line.find_first_of(':');
        
        string stop_name = line.substr(0, pos_end_name);
        auto split_coor = line.find_first_of(',');

        // проверка на запрос информации по остановке
        if (split_coor == std::string::npos) {
                
            auto info = cat.GetStopInfo (stop_name); 
            transcat::output::DateOutput (output_cout, info);
          
        } 
        else {
            line = line.substr(pos_end_name+1); 

            split_coor = line.find_first_of(',');
            std::string x_coor = line.substr(0, (split_coor));
            line = line.substr(split_coor+1); 

            split_coor = line.find_first_of(',');
            std::string y_coor = line.substr(0, (split_coor));
            line = line.substr(split_coor+1); 

            std::deque <std::pair <string, int>> dis;
            while (line.size() > 0) {
                split_coor = line.find_first_of('m');
                if (split_coor == std::string::npos) {break;}
                std::string d1 = line.substr(0, (split_coor));
                line = line.substr(split_coor+5); 
                split_coor = line.find_first_of(',');
                if (split_coor == std::string::npos) {
                    std::string stop_name_d1 = line.substr(0);
                    dis.push_back({stop_name_d1, std::stoi(d1)});
                    break;
                } 
                else {
                    std::string stop_name_d1 = line.substr(0, (split_coor));
                    dis.push_back({stop_name_d1, std::stoi(d1)});
                    line = line.substr(split_coor+1); 
                }
                    
            }
            
            double x = std::stod(x_coor);
            double y = std::stod(y_coor);
            
            result.name = stop_name;
            result.xy.lat = x;
            result.xy.lng = y;

            cat.AddStop(move(result));
            for (size_t i = 0; i < dis.size(); ++i) {
                auto d_a = cat.FindStop (stop_name);
                auto d_b = cat.FindStop (dis[i].first);
                int tempor = dis[i].second;
                if (d_a == nullptr || d_b == nullptr) {
                        dist_waiting_add.push_back({stop_name, dis[i].first, tempor});  
                }
                else {
                    cat.InputDistance (d_a, d_b, dis[i].second);    
                }
            }
        }

}


void Load(transcat::TransportCatalogue& cat, std::istream& input, std::ostream& output_cout) {
    while (input.peek() != EOF){ 
        std::string num;
        std::getline(input, num);
        int i = std::stoi( move(num) );
        std::deque <std::pair <Bus, std::deque<string>>> buses_wait_add;
        std::deque <std::tuple <string, string, int>> dist_waiting_add;
        for (std::string line; std::getline(input, line);) {
            
            if (line.size() > 4) {
                
                std::string type_op = line.substr(0, 4);
            
                if (type_op == "Bus "s) {
                    BusInput (output_cout, cat, buses_wait_add, line);
                } 
                else if (type_op == "Stop"s) {
                    StopInput (output_cout, cat,dist_waiting_add, line);
                }
            }
            if (i == 1) {break;}
            --i;
            
        }
            if (buses_wait_add.size() > 0) {
                for (auto& bus_r:buses_wait_add) {
                    auto dfgd = CheckStops (cat, bus_r.second, bus_r.first);
                    if (dfgd) {
                        cat.AddBusRoute (move(bus_r.first));
                    }
                }
            }
            if (dist_waiting_add.size() > 0) {
                for (auto& dist_t : dist_waiting_add) {
                    cat.InputDistance (cat.FindStop (std::get<0>(dist_t)), 
                                       cat.FindStop (std::get<1>(dist_t)), std::get<2>(dist_t));    
                   
                }
            }
    } // конец цикла while
} 

} //конец namespace transcat::input

