#pragma once
/*
 * Класс транспортного справочника. Выполняет следующие задачи:
 * 1) добавление маршрута в базу (AddBus),
 * 2) добавление остановки в базу (AddStop),
 * 3) поиск маршрута по имени (FindBus),
 * 4) поиск остановки по имени (FindStop),
 * 5) получение информации о маршруте (GetBusStatistics),
 * 6) получение сортированного списка автобусов, проходящих через остановку (GetStopStatistics).
 * 7) задание дистанции между остановками (AddStopDistances) (Расстояние между остановками A и B может быть как не равно,
 * так и равно расстоянию между остановками B и A. В первом случае расстояние между остановками указывается дважды:
 * в прямом и в обратном направлении. Когда расстояния одинаковы достаточно задать расстояние от A до B, либо расстояние от B до A.
 * Также разрешается задать расстояние от остановки до самой себя — так бывает, если автобус разворачивается и приезжает
 * на ту же остановку.),
 * 8) получение длины маршрута (CalculateTotalDistance) по географическим координатам и по расстояниям между остановками.
 * Методы класса TransportCatalogue не должны выполнять никакого ввода-вывода.
 */

#include "domain.h"
#include "geo.h"

#include <deque>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace transport {
    namespace catalogue {

        struct DistanceBetweenStops {
            double geographic;
            size_t measured;
        };

        class TransportCatalogue {
        public:
            struct StopPointerHasher {
                size_t operator()(const std::pair<Stop *, Stop *> &stops) const {
                    std::hash<const void *> ptr_hasher;
                    return static_cast<size_t>(ptr_hasher(stops.first)) + static_cast<size_t>(ptr_hasher(stops.second)) * 37;
                }
            };

            void AddStop(std::string_view stop_name, geo::Coordinates location);

            void AddStopDistances(std::string_view stop_name_from, std::string_view stop_name_to, size_t dist);

            void AddBus(std::string_view bus_name, const std::vector<std::string_view> &stops, bool is_roundtrip = false);

            Stop *FindStop(const std::string_view stop_name) const;

            Bus *FindBus(const std::string_view bus_name) const;

            std::optional<BusStatistics> GetBusStatistics(const std::string_view bus_name) const;

            std::optional<std::vector<std::string>> GetStopStatistics(const std::string_view stop_name) const;

            std::vector<std::string> GetAllStopNames() const;

            std::vector<std::string> GetAllBusNames() const;

            /* Расстояние между остановками исключительно рядом стоящими */
            size_t GetDistanceBetwenStops(Stop *stop_from, Stop *stop_to) const;

            size_t GetStopCount() const;

        private:
            std::deque<Stop> stops_;
            std::deque<Bus> buses_;
            std::unordered_map<std::string_view, Stop *> stop_names_;
            std::unordered_map<std::string_view, Bus *> bus_names_;

            std::unordered_map<std::string_view, std::unordered_set<std::string>> buses_for_stop_;
            std::unordered_map<std::pair<Stop *, Stop *>, size_t, StopPointerHasher> stops_dist_; // расстояние между остановками: остановка "откуда", остановка "куда"

            DistanceBetweenStops CalculateTotalDistance(const Bus *bus) const; // возвращает длины маршрута: по географическим координатам и по расстояниям между остановками
        };
    } // конец namespace catalogue
} // конец namespace transport