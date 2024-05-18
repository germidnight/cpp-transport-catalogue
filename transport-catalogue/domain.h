#pragma once
/*
 * Структуры:
 * 1) Остановка, каждая запись об остановке содержит:
 * - координаты остановки: широту и долготу;
 * - имя остановки;
 * 2) Информация о маршруте (для выдачи статистики):
 * - географическая длина маршрута;
 * - извилистость, то есть отношение фактической длины маршрута к географическому расстоянию;
 * - дорожная длина маршрута;
 * - количество остановок;
 * - количество уникальных остановок;
 * 3) Автобус, каждая запись о маршруте содержит:
 * - информацию о маршруте (для выдачи статистики)
 * - имя маршрута;
 * - последовательный список всех остановок маршрута;
 */
#include "geo.h"

#include <string>
#include <vector>

namespace transport {
    namespace catalogue {

        struct Stop {
            geo::Coordinates location;
            std::string name;

            [[nodiscard]] bool operator!=(const Stop &rhs) const noexcept;
            [[nodiscard]] bool operator==(const Stop &rhs) const noexcept;
        };

        struct BusStatistics {
            double length = 0;
            double curvature = 1;
            size_t distance = 0;
            size_t stops_num = 0;
            size_t uniq_stops_num = 0;
        };

        struct Bus {
            BusStatistics bus_stat;
            std::string name;
            std::vector<Stop *> stops;
            bool is_roundtrip_;

            [[nodiscard]] bool operator!=(const Bus &rhs) const noexcept;
            [[nodiscard]] bool operator==(const Bus &rhs) const noexcept;
        };
    } // namespace catalogue
} //namespace transport