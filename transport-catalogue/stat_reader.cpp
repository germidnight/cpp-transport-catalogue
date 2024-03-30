/* обработка запросов к базе и вывод данных */
#include <algorithm>
#include <iomanip>
#include <iostream>

#include "stat_reader.h"

using namespace std::literals;
using namespace transport::catalogue;

namespace transport {
namespace stat_reader {

void ParseAndPrintStat(const TransportCatalogue &transport_catalogue, std::string_view request,
                       std::ostream &output) {
    std::string_view name;

    const size_t com_pos = request.find_first_not_of(' ');
    size_t pos = request.find_first_of(' ', com_pos);
    auto command = request.substr(com_pos, pos - com_pos);
    name = request.substr(pos + 1);
    if (command == "Bus"s) {
        BusStatistics bus_stat = transport_catalogue.GetBusStatistics(name);
        if(bus_stat.name != ""s) {
            output << std::setprecision(6)
                   << "Bus "s << bus_stat.name << ": "s << bus_stat.stops_num << " stops on route, "s
                   << bus_stat.uniq_stops_num << " unique stops, "s << bus_stat.length << " route length"s << std::endl;
        } else {
            output << "Bus "s << name << ": not found"s << std::endl;
        }
    } else if(command == "Stop"s) {
        auto [is_found, buses] = transport_catalogue.GetStopStatistics(name);
        if(is_found == FindStopResult::FOUND) {
            output << "Stop "s << name << ": buses"s;
            for (const std::string &bus : buses) {
                output << " "s << bus;
            }
            output << std::endl;
        } else if(is_found == FindStopResult::EMPTY) {
            output << "Stop "s << name << ": no buses"s << std::endl;
        } else {
            output << "Stop "s << name << ": not found"s << std::endl;
        }
    }

}
} // конец namespace stat_reader
} // конец namespace transport