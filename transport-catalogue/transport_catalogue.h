/* класс транспортного справочника
 * Должен иметь методы для выполнения следующих задач:
 * 1) добавление маршрута в базу,
 * 2) добавление остановки в базу,
 * 3) поиск маршрута по имени,
 * 4) поиск остановки по имени,
 * 5) получение информации о маршруте,
 * 6) получение сортированного списка автобусов, проходящих через остановку.
 * 7) задание дистанции между остановками (Расстояние между остановками A и B может быть как не равно,
 * так и равно расстоянию между остановками B и A. В первом случае расстояние между остановками указывается дважды:
 * в прямом и в обратном направлении. Когда расстояния одинаковы достаточно задать расстояние от A до B, либо расстояние от B до A.
 * Также разрешается задать расстояние от остановки до самой себя — так бывает, если автобус разворачивается и приезжает
 * на ту же остановку.),
 * 8) получение дистанции между остановками.
 * Методы класса TransportCatalogue не должны выполнять никакого ввода-вывода.
 */
#pragma once

#include <deque>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace transport {
namespace catalogue {
    /* Каждая запись об остановке содержит:
     * - координаты остановки: широту и долготу;
     * - имя остановки;
     * - множество автобусных маршрутов, проходящих через остановку для быстрого поиска по имени маршрута
     * - расстояния между автобусными остановками (от остановки, до остановки)
     */
    struct Stop {
        struct StopPointerHasher {
            size_t operator()(const std::pair<Stop *, Stop *> &stops) const {
                std::hash<const void *> ptr_hasher;
                return static_cast<size_t>(ptr_hasher(stops.first)) + static_cast<size_t>(ptr_hasher(stops.second)) * 37;
            }
        };

        double latitude;
        double longitude;
        std::string name;
        std::unordered_set<std::string> buses;
        std::unordered_map<std::pair<Stop*, Stop*>, size_t, StopPointerHasher> stops_dist; // расстояние между остановками: остановка "откуда", остановка "куда"

        [[nodiscard]] bool operator!=(const Stop &rhs) const noexcept;
        [[nodiscard]] bool operator==(const Stop &rhs) const noexcept;
    };
    std::ostream &operator<<(std::ostream &output, const Stop &stop);

    enum class FindStopResult {
        FOUND,
        NOT_FOUND,
        EMPTY
    };

    /* Каждая запись о маршруте содержит:
     * - длину маршрута по географическим координатам (вычисляется в момент первого запроса этой информации);
     * - длину маршрута по расстояниям между остановками (вычисляется в момент первого запроса этой информации);
     * - имя маршрута;
     * - последовательный список всех остановок маршрута;
     * - множество остановок на маршруте для быстрого поиска по имени остановки
     */
    struct Bus {
        double length = 0; // обновляем при первом запросе маршрута
        size_t distance = 0;
        std::string name;
        std::deque<Stop *> stops;
        std::unordered_set<std::string> stop_names;

        [[nodiscard]] bool operator!=(const Bus &rhs) const noexcept;
        [[nodiscard]] bool operator==(const Bus &rhs) const noexcept;
    };
    std::ostream &operator<<(std::ostream &output, const Bus &bus);

    /* Информация о маршруте:
     * - географическая длина маршрута;
     * - извилистость, то есть отношение фактической длины маршрута к географическому расстоянию;
     * - дорожная длина маршрута;
     * - количество остановок;
     * - количество уникальных остановок;
     * - имя остановки.
     */
    struct BusStatistics {
        double length = 0;
        double curvature = 1;
        size_t distance = 0;
        size_t stops_num = 0;
        size_t uniq_stops_num = 0;
        std::string name;
    };

    class TransportCatalogue {
    public:
        void AddStop(std::string_view stop_name, double latitude, double longitude);

        void AddStopDistances(std::string_view stop_name_from, std::vector<std::pair<std::string, size_t>> &&dist_to_stops);

        void AddBus(std::string_view bus_name, const std::vector<std::string_view> &stops);

        Stop FindStop(const std::string_view stop_name) const;

        Bus FindBus(const std::string_view bus_name) const;

        BusStatistics GetBusStatistics(const std::string_view bus_name) const;

        std::pair<FindStopResult, std::vector<std::string>> GetStopStatistics(const std::string_view stop_name) const;

    private:
        std::deque<Stop> stops_;
        std::deque<Bus> buses_;
        std::unordered_map<std::string_view, Stop *> stop_names_;
        std::unordered_map<std::string_view, Bus *> bus_names_;

        std::pair<double, size_t> CalculateTotalDistance(const Bus &bus) const; // возвращает длины маршрута: по географическим координатам и по расстояниям между остановками
    };
} // конец namespace catalogue
} // конец namespace transport