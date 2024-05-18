/* класс транспортного справочника */
#include "transport_catalogue.h"

#include <algorithm>
#include <assert.h>
#include <iostream>
#include <iterator>
#include <utility>

namespace transport {
    namespace catalogue {

        void TransportCatalogue::AddStop(std::string_view stop_name, geo::Coordinates location) {
            stops_.push_back({location, std::string(stop_name)});
            stop_names_[stops_.back().name] = &stops_.back();
        }

        void TransportCatalogue::AddStopDistances(std::string_view stop_name_from, std::string_view stop_name_to, size_t distance) {
            Stop *stop_from = nullptr;
            Stop *stop_to = nullptr;
            if (stop_names_.count(stop_name_from) > 0) {
                stop_from = stop_names_[stop_name_from];
            } else {
                return;
            }

            if (stop_names_.count(stop_name_to) > 0) {
                stop_to = stop_names_[stop_name_to];
            } else {
                return;
            }
            stops_dist_[{stop_from, stop_to}] = distance;
        }

        void TransportCatalogue::AddBus(std::string_view bus_name, const std::vector<std::string_view> &stops, bool is_roundtrip) {
            Bus bus;
            bus.bus_stat = {};
            bus.name = std::string(bus_name);
            bus.is_roundtrip_ = is_roundtrip;
            for (auto stop : stops) {
                buses_for_stop_[FindStop(stop)->name].insert(bus.name);
                Stop *cur_stop = stop_names_.at(stop);
                bus.stops.push_back(cur_stop);
            }
            buses_.push_back(std::move(bus));
            bus_names_[buses_.back().name] = &buses_.back();
        }

        Stop *TransportCatalogue::FindStop(const std::string_view stop_name) const {
            if (stop_names_.count(stop_name) > 0) {
                return stop_names_.at(stop_name);
            }
            return nullptr;
        }

        Bus *TransportCatalogue::FindBus(const std::string_view bus_name) const {
            if (bus_names_.count(bus_name) > 0) {
                return bus_names_.at(bus_name);
            }
            return nullptr;
        }

        std::optional<BusStatistics> TransportCatalogue::GetBusStatistics(const std::string_view bus_name) const {
            Bus *bus = FindBus(bus_name);
            if (bus != nullptr) {
                if (bus->bus_stat.length == 0) {
                    const auto res = CalculateTotalDistance(bus);
                    bus->bus_stat.length = res.first;
                    bus->bus_stat.distance = res.second;
                    bus->bus_stat.curvature = static_cast<double>(bus->bus_stat.distance) / bus->bus_stat.length;
                    bus->bus_stat.stops_num = bus->stops.size();

                    std::unordered_set<std::string> unique_stops;
                    for (const Stop *stop : bus->stops) {
                        unique_stops.insert(stop->name);
                    }
                    bus->bus_stat.uniq_stops_num = unique_stops.size();
                }
                return bus->bus_stat;
            }
            return std::nullopt;
        }

        std::pair<double, size_t> TransportCatalogue::CalculateTotalDistance(const Bus *bus) const {
            using namespace geo;

            double length = 0;
            size_t distance = 0;

            Coordinates geo_from_stop, geo_to_stop;
            geo_from_stop = {bus->stops.front()->location.lat, bus->stops.front()->location.lng};

            Stop *stop_from, *stop_to;
            stop_from = bus->stops[0];

            for (size_t idx = 1; idx != bus->stops.size(); ++idx) {
                stop_to = bus->stops[idx];
                if (stops_dist_.count({stop_from, stop_to}) > 0) {
                    distance += stops_dist_.at({stop_from, stop_to});
                } else if (stops_dist_.count({stop_to, stop_from}) > 0) {
                    distance += stops_dist_.at({stop_to, stop_from});
                }

                geo_to_stop = {stop_to->location.lat, stop_to->location.lng};
                length += ComputeDistance(geo_from_stop, geo_to_stop);
                geo_from_stop = geo_to_stop;

                stop_from = stop_to;
            }
            return {length, distance};
        }

        std::pair<FindStopResult, std::vector<std::string>> TransportCatalogue::GetStopStatistics(const std::string_view stop_name) const {
            std::vector<std::string> buses;
            if (FindStop(stop_name) != nullptr) {
                if (buses_for_stop_.count(stop_name) > 0) {
                    for (std::string_view bus : buses_for_stop_.at(stop_name)) {
                        buses.push_back(std::string(bus));
                    }
                    std::sort(buses.begin(), buses.end());
                    return std::make_pair(FindStopResult::FOUND, buses);
                } else {
                    return std::make_pair(FindStopResult::EMPTY, buses);
                }
            }
            return std::make_pair(FindStopResult::NOT_FOUND, buses);
        }

        /* Перебираем все остановки и наполняем вектор имён всех остановок с маршрутами */
        std::unique_ptr<std::vector<std::string>> TransportCatalogue::GetAllStopNames() const {
            std::vector<std::string> all_stop_names;
            all_stop_names.reserve(stops_.size());
            for(const Stop& stop : stops_) {
                if (buses_for_stop_.count(stop.name) && !buses_for_stop_.at(stop.name).empty()) {
                    all_stop_names.emplace_back(stop.name);
                }
            }
            return std::make_unique<std::vector<std::string>>(std::move(all_stop_names));
        }

        /* Перебираем все маршруты и наполняем вектор имён всех маршрутов */
        std::unique_ptr<std::vector<std::string>> TransportCatalogue::GetAllBusNames() const {
            std::vector<std::string> all_bus_names;
            all_bus_names.reserve(buses_.size());
            for(const Bus &bus : buses_) {
                all_bus_names.emplace_back(bus.name);
            }
            return std::make_unique<std::vector<std::string>>(std::move(all_bus_names));
        }
    } // конец namespace catalogue
} // конец namespace transport