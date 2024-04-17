/* обработка запросов к базе и вывод данных */
#include <algorithm>
#include <iomanip>
#include <iostream>

#include "stat_reader.h"

using namespace std::literals;
using namespace transport::catalogue;

namespace transport {
namespace stat_reader {

void PrintBusInfo(const TransportCatalogue& transport_catalogue, std::string_view bus_name, std::ostream &output) {
    std::optional<BusStatistics> bus_stat = transport_catalogue.GetBusStatistics(bus_name);
    if (bus_stat.has_value()) {
        output << std::setprecision(6)
               << "Bus "s << bus_name << ": "s << bus_stat.value().stops_num << " stops on route, "s
               << bus_stat.value().uniq_stops_num << " unique stops, "s << static_cast<double>(bus_stat.value().distance) << " route length, "s
               << bus_stat.value().curvature << " curvature" << std::endl;
    } else {
        output << "Bus "s << bus_name << ": not found"s << std::endl;
    }
}

void PrintStopInfo(const TransportCatalogue& transport_catalogue, std::string_view stop_name, std::ostream& output) {
    auto [is_found, buses] = transport_catalogue.GetStopStatistics(stop_name);
    if (is_found == FindStopResult::FOUND) {
        output << "Stop "s << stop_name << ": buses"s;
        for (const std::string &bus : buses) {
            output << " "s << bus;
        }
        output << std::endl;
    } else if (is_found == FindStopResult::EMPTY) {
        output << "Stop "s << stop_name << ": no buses"s << std::endl;
    } else {
        output << "Stop "s << stop_name << ": not found"s << std::endl;
    }
}

void ParseAndPrintStat(const TransportCatalogue &transport_catalogue, std::string_view request,
                       std::ostream &output) {
    std::string_view name;

    const size_t com_pos = request.find_first_not_of(' ');
    size_t pos = request.find_first_of(' ', com_pos);
    auto command = request.substr(com_pos, pos - com_pos);
    name = request.substr(pos + 1);
    if (command == "Bus"s) {
        PrintBusInfo(transport_catalogue, name, output);
    } else if(command == "Stop"s) {
        PrintStopInfo(transport_catalogue, name, output);
    }
}

void ProcessRequests2Catalogue(std::istream& input, std::ostream& output, int stat_request_count,
                                const TransportCatalogue& catalogue) {
    for (int i = 0; i < stat_request_count; ++i) {
        std::string line;
        std::getline(input, line);
        ParseAndPrintStat(catalogue, line, output);
    }
}

std::ostream &operator<<(std::ostream &output, const Bus &bus) {
    output << "BUS " << bus.name << ":"s;
    for (const auto &stop : bus.stops) {
        output << " "s << stop->name;
    }
    output << std::endl;
    return output;
}

std::ostream &operator<<(std::ostream &output, const Stop &stop) {
    output << "STOP " << stop.name << ": "s << stop.location.lat << " "s << stop.location.lng << " "s;
    return output;
}

} // конец namespace stat_reader
} // конец namespace transport