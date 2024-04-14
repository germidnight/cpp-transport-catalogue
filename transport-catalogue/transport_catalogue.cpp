/* класс транспортного справочника */
#include "geo.h"
#include "transport_catalogue.h"

#include <algorithm>
#include <assert.h>
#include <iostream>
#include <iterator>
#include <utility>

using namespace std::literals;

namespace transport {
namespace catalogue {

    void TransportCatalogue::AddStop(std::string_view stop_name, double latitude, double longitude) {
        stops_.push_back({latitude, longitude, std::string(stop_name), {}, {}});
        stop_names_[stops_.back().name] = &stops_.back();
    }

    void TransportCatalogue::AddStopDistances(std::string_view stop_name_from, std::vector<std::pair<std::string, size_t>> &&dist_to_stops) {
        Stop* stop_from = nullptr;
        Stop* stop_to = nullptr;
        if(stop_names_.count(stop_name_from) > 0) {
            stop_from = stop_names_[stop_name_from];
        } else {
            assert("Остановка (откуда) не найдена");
            return;
        }
        for(auto [stop_name_to, distance] : dist_to_stops) {
            if (stop_names_.count(stop_name_to) > 0) {
                stop_to = stop_names_[stop_name_to];
            } else {
                //assert("Остановка (куда) не найдена");
                continue;
            }
            stop_from->stops_dist[{stop_from, stop_to}] = distance;
        }
    }

    void TransportCatalogue::AddBus(std::string_view bus_name, const std::vector<std::string_view>& stops) {
        Bus bus;
        bus.length = 0;
        bus.distance = 0;
        bus.name = std::string(bus_name);
        for(auto stop : stops) {
            bus.stop_names.insert(std::string(stop));
            Stop* cur_stop = stop_names_.at(stop);
            bus.stops.push_back(cur_stop);
            cur_stop->buses.insert(bus.name);
        }
        buses_.push_back(std::move(bus));
        bus_names_[buses_.back().name] = &buses_.back();
    }

    Stop TransportCatalogue::FindStop(const std::string_view stop_name) const {
        if(stop_names_.count(stop_name) > 0) {
            return *(stop_names_.at(stop_name));
        }
        return {};
    }

    Bus TransportCatalogue::FindBus(const std::string_view bus_name) const {
        if(bus_names_.count(bus_name) > 0) {
            return *(bus_names_.at(bus_name));
        }
        return {};
    }

    BusStatistics TransportCatalogue::GetBusStatistics(const std::string_view bus_name) const {
        Bus bus = FindBus(bus_name);
        if(bus.name != ""s) {
            if(bus.length == 0) {
                const auto res = CalculateTotalDistance(bus);
                bus.length = res.first;
                bus.distance = res.second;
            }
            return {bus.length, static_cast<double>(bus.distance) / bus.length, bus.distance,
                            bus.stops.size(), bus.stop_names.size(), bus.name};
        }
        return {};
    }

    std::pair<double, size_t> TransportCatalogue::CalculateTotalDistance(const Bus& bus) const {
        using namespace detail;

        double length = 0;
        size_t distance = 0;

        Coordinates geo_from_stop, geo_to_stop;
        geo_from_stop = {bus.stops.front()->latitude, bus.stops.front()->longitude};

        Stop *stop_from, *stop_to;
        stop_from = bus.stops[0];

        for(size_t idx = 1; idx != bus.stops.size(); ++idx) {
            stop_to = bus.stops[idx];
            if(stop_from->stops_dist.count({stop_from, stop_to}) > 0) {
                distance += stop_from->stops_dist[{stop_from, stop_to}];
            } else if(stop_to->stops_dist.count({stop_to, stop_from}) > 0) {
                distance += stop_to->stops_dist[{stop_to, stop_from}];
            }

            geo_to_stop = {stop_to->latitude, stop_to->longitude};
            length += ComputeDistance(geo_from_stop, geo_to_stop);
            geo_from_stop = geo_to_stop;

            stop_from = stop_to;
        }
        return {length, distance};
    }

    std::pair<FindStopResult, std::vector<std::string>> TransportCatalogue::GetStopStatistics(const std::string_view stop_name) const {
        std::vector<std::string> buses;
        Stop res_stop = FindStop(stop_name);
        if(res_stop.name != ""s) {
            for(const std::string& bus : res_stop.buses) {
                buses.push_back(bus);
            }
            if(!buses.empty()) {
                std::sort(buses.begin(), buses.end());
                return std::make_pair(FindStopResult::FOUND, buses);
            } else {
                return std::make_pair(FindStopResult::EMPTY, buses);
            }
        }
        return std::make_pair(FindStopResult::NOT_FOUND, buses);
    }

    [[nodiscard]] bool Stop::operator!=(const Stop &rhs) const noexcept {
        return (this->latitude != rhs.latitude) || (this->longitude != rhs.longitude) || (this->name != rhs.name) || (this->buses != rhs.buses)
         || (this->stops_dist != rhs.stops_dist);
    }
    [[nodiscard]] bool Stop::operator==(const Stop &rhs) const noexcept {
        return !(*this != rhs);
    }

    std::ostream& operator<<(std::ostream& output, const Stop& stop) {
        output << "STOP " << stop.name << ": "s << stop.latitude << " "s << stop.longitude << " "s;
        return output;
    }

    [[nodiscard]] bool Bus::operator!=(const Bus &rhs) const noexcept {
        if(this->name != rhs.name) {
            return true;
        }
        if(this->stops.size() != rhs.stops.size()) {
            return true;
        }
        auto it_rhs = rhs.stops.begin();
        for(auto it_lhs = this->stops.begin(); it_lhs != this->stops.end(); ++it_lhs, ++it_rhs) {
            if((*it_lhs)->name != (*it_rhs)->name) {
                return true;
            }
        }
        return false;
    }
    [[nodiscard]] bool Bus::operator==(const Bus &rhs) const noexcept {
        return !(*this != rhs);
    }

    std::ostream &operator<<(std::ostream &output, const Bus &bus) {
        output << "BUS " << bus.name << ":"s;
        for(const auto& stop : bus.stops) {
            output << " "s << stop->name;
        }
        output << std::endl;
        return output;
    }
} // конец namespace tcatalogue
} // конец namespace transport