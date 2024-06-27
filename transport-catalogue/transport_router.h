#pragma once
/*
 * Класс и структуры для создания графа транспортного каталога и разбора полученного оптимального пути
 * Создание графа:
 * 1) Построить граф по транспортному каталогу, заполнить таблицы соответствий:
 *  - вершины графа (graph::VertexId) <-> остановки (ransport::catalogue::Stop*) (каждая остановка учитывается дважды)
 *  - рёбра графа (graph::EdgeId) <-> маршруты (ransport::catalogue::Bus*) (все возможные пути включая высадку на промежуточных остановках)
 * 2) построить маршрут на графе, вызвав graph::Router::BuildRoute()
 * 3) вернуть маршрут на транспортном каталоге
 *
 * Две вершины для каждой из остановок:
 * - первая соответствует состоянию «начал ждать автобус на остановке S»,
 * - вторая — «садится в автобус на остановке S».
 * Время ожидания автобуса учитывается благодаря ребру веса bus_wait_time из первой вершины во вторую.
 *
 * Основная идея в том, что нужно строить ребра "насквозь" в рамках одного маршрута, чтобы не получить лишнего ожидания на остановках,
 * которые между начальной и конечной.
 */
#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

namespace transport_router {

    struct RouterSetting {
        int bus_wait_time;
        double bus_velocity;
    };

    struct FoundRouteResult {
        struct Wait {
            std::string stop;
            double time;
        };
        struct Bus {
            size_t span_count;
            std::string bus;
            double time;
        };

        double total_time;
        std::vector<std::variant<Wait, Bus>> route;
    };

    class RouteBuilder {
    public:
        explicit RouteBuilder(const transport::catalogue::TransportCatalogue& transport_catalogue, const RouterSetting& settings);
        std::optional<FoundRouteResult> FindRoute(std::string_view from_station, std::string_view to_station) const;
        ~RouteBuilder() {
            if (graph_ != nullptr) {delete(graph_);}
            if (router_ != nullptr) {delete(router_);}
        }

    private:
        void VertexFill();
        void EdgesFill();
        /* Внесение рёбер графа. Подаём на вход итераторы на начало и конец диапазона остановок, указатель на автобус*/
        template <typename IterCatalogueStops>
        void InsertEdgesForRoute(IterCatalogueStops begin_it, IterCatalogueStops end_it, transport::catalogue::Bus* bus) {
            using namespace graph;
            using namespace transport::catalogue;

            for (auto from_it = begin_it; from_it != std::prev(end_it); ++from_it) {
                VertexId from_vid = vertexes_.at(*from_it);
                double edge_weight = 0.;
                Stop *old_from_stop = *from_it;
                for (auto to_it = std::next(from_it); to_it != end_it; ++to_it) {
                    VertexId to_vid = vertexes_.at(*to_it);
                    size_t distance_betwen_stops = transport_catalogue_.GetDistanceBetwenStops(old_from_stop, *to_it);
                    if (distance_betwen_stops == 0) {
                        distance_betwen_stops = transport_catalogue_.GetDistanceBetwenStops(*to_it, old_from_stop);
                    }
                    edge_weight += static_cast<double>(distance_betwen_stops) / router_settings_.bus_velocity;
                    buses_[graph_->AddEdge({from_vid, to_vid + 1, edge_weight})] = bus; // рёбра в нечётные вершины - сюда приезжают автобусы
                    old_from_stop = *to_it;
                }
            }
        }
        bool IsStopValid(std::string_view stop) const;

        const transport::catalogue::TransportCatalogue& transport_catalogue_;
        const RouterSetting& router_settings_;
        graph::DirectedWeightedGraph<double>* graph_ = nullptr;
        graph::Router<double>* router_ = nullptr;
        std::vector<transport::catalogue::Stop*> stops_;
        /* Если будет много операций построения маршрута, то в хэше vertexes_ ключ можно попробовать поменять на string */
        std::unordered_map<transport::catalogue::Stop*, graph::VertexId> vertexes_; /* только для чётных вершин графа - отсюда выезжают автобусы */
        std::unordered_map<graph::EdgeId, std::optional<transport::catalogue::Bus*>> buses_; /* рёбра ожидания на остановке имеют значение nullopt */
    };

} // namespace transport_router