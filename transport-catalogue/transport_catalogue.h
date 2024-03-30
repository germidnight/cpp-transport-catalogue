/* класс транспортного справочника
 * Должен иметь методы для выполнения следующих задач:
 * 1) добавление маршрута в базу,
 * 2) добавление остановки в базу,
 * 3) поиск маршрута по имени,
 * 4) поиск остановки по имени,
 * 5) получение информации о маршруте,
 * 6) получение сортированного списка автобусов, проходящих через остановку.
 * Методы класса TransportCatalogue не должны выполнять никакого ввода-вывода.
 */
#pragma once

#include <deque>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace transport {
namespace catalogue {
    /* Каждая запись об остановке содержит:
     * - координаты остановки: широту и долготу;
     * - имя остановки;
     * - множество автобусных маршрутов, проходящих через остановку для быстрого поиска по имени маршрута
     */
    struct Stop {
        double latitude;
        double longitude;
        std::string name;
        std::unordered_set<std::string> buses;

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
     * - длину маршрута (вычисляется в момент первого запроса этой информации);
     * - имя маршрута;
     * - последовательный список всех остановок маршрута;
     * - множество остановок на маршруте для быстрого поиска по имени остановки
     */
    struct Bus {
        double length = 0; // обновляем при первом запросе маршрута
        std::string name;
        std::deque<Stop *> stops;
        std::unordered_set<std::string> stop_names;

        [[nodiscard]] bool operator!=(const Bus &rhs) const noexcept;
        [[nodiscard]] bool operator==(const Bus &rhs) const noexcept;
    };
    std::ostream &operator<<(std::ostream &output, const Bus &bus);

    /* Информация о маршруте:
     * - длина маршрута;
     * - количество остановок;
     * - количество уникальных остановок;
     * - имя остановки.
     */
    struct BusStatistics {
        double length = 0;
        size_t stops_num = 0;
        size_t uniq_stops_num = 0;
        std::string name;
    };

    class TransportCatalogue {
    public:
        void AddStop(std::string_view stop_name, double latitude, double longitude);

        void AddBus(std::string_view bus_name, std::vector<std::string_view> stops);

        Stop FindStop(const std::string_view stop_name) const;

        Bus FindBus(const std::string_view bus_name) const;

        BusStatistics GetBusStatistics(const std::string_view bus_name) const;

        std::pair<FindStopResult, std::vector<std::string>> GetStopStatistics(const std::string_view stop_name) const;

    private:
        std::deque<Stop> stops_;
        std::deque<Bus> buses_;
        std::unordered_map<std::string_view, Stop *> stop_names_;
        std::unordered_map<std::string_view, Bus *> bus_names_;

        double CalculateTotalDistance(const Bus &bus) const;
    };
} // конец namespace catalogue
} // конец namespace transport