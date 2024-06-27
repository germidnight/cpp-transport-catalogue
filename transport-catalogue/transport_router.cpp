#include "transport_router.h"

#include <cassert>
#include <iterator>

namespace transport_router {
    using namespace std;
    using namespace graph;
    using namespace transport::catalogue;
    /*
     * Построить граф по транспортному каталогу, заполнить таблицы соответствий:
     *  - вершины графа (graph::VertexId) <-> остановки (ransport::catalogue::Stop*) (каждая остановка учитывается дважды)
     *  - рёбра графа (graph::EdgeId) <-> маршруты (ransport::catalogue::Bus*) (все возможные пути включая высадку на промежуточных остановках)
     *
     * Граф строим за 2 итерации:
     * 1) добавление вершин, добавление рёбер ожиданий,
     * 2) добавление рёбер маршрутов.
     */
    RouteBuilder::RouteBuilder(const TransportCatalogue &transport_catalogue, const RouterSetting &settings)
                : transport_catalogue_(transport_catalogue), router_settings_(settings) {
        size_t data_size = transport_catalogue_.GetStopCount() * 2;
        stops_.resize(data_size, nullptr);
        graph_ = new DirectedWeightedGraph<double>(data_size);
        VertexFill();
        EdgesFill();

        router_ = new Router<double>(*graph_);
    }

    void RouteBuilder::VertexFill() {
        vector<string> all_stop_names = transport_catalogue_.GetAllStopNames();
        VertexId vid = 0;
        for (auto it = all_stop_names.begin(); it != all_stop_names.end(); ++it) {
            Stop *temp_stop = transport_catalogue_.FindStop(*it);
            /* чётные вершины - для маршрутов, т.е. отсюда выезжают автобусы */
            stops_[vid] = temp_stop;
            vertexes_[temp_stop] = vid;
            ++vid;
            /* нечётные вершины - для ожидания на остановке, т.е. сюда приезжают автобусы */
            stops_[vid] = temp_stop;
            ++vid;
            /* добавляем рёбра для ожидания на остановке, направление рёбер: из куда приезжают автобусы -> в откуда уезжают*/
            buses_[graph_->AddEdge({vid - 1, vid - 2, static_cast<double>(router_settings_.bus_wait_time)})] = nullopt;
        }
    }

    /*
     * На конечных остановках все автобусы высаживают пассажиров и уезжают в парк.
     * Даже если человек едет на кольцевом — "is_roundtrip": true — маршруте и хочет проехать мимо конечной,
     * он будет вынужден выйти и подождать тот же самый автобус ровно bus_wait_time минут.
     */
    void RouteBuilder::EdgesFill() {
        vector<string> all_bus_names = transport_catalogue_.GetAllBusNames();

        for (auto it = all_bus_names.begin(); it != all_bus_names.end(); ++it) {
            Bus *bus = transport_catalogue_.FindBus(*it);

            if (bus->stops.begin() == bus->stops.end()) {
                continue; // в граф не включаются маршруты без остановок
            }
            if (next(bus->stops.begin()) == bus->stops.end()) {
                continue; // в граф не включаются маршруты из одной остановки
            }

            if (bus->is_roundtrip_) {
                InsertEdgesForRoute(bus->stops.begin(), bus->stops.end(), bus);
            } else {
                auto konechnaya_it = prev(bus->stops.end(), static_cast<long long int>(bus->stops.size() / 2));
                InsertEdgesForRoute(bus->stops.begin(), konechnaya_it, bus);
                InsertEdgesForRoute(prev(konechnaya_it), bus->stops.end(), bus);
            }
        }
    }

    bool RouteBuilder::IsStopValid(std::string_view stop_name) const {
        Stop *stop = transport_catalogue_.FindStop(stop_name);
        if ((stop != nullptr) && (vertexes_.count(stop) > 0)) {
            return true;
        }
        return false;
    }

    std::optional<FoundRouteResult> RouteBuilder::FindRoute(std::string_view from_station, std::string_view to_station) const {
        if (!IsStopValid(from_station) || !IsStopValid(to_station)) {
            return nullopt;
        }

        VertexId from_vid = vertexes_.at(transport_catalogue_.FindStop(from_station));
        VertexId to_vid = vertexes_.at(transport_catalogue_.FindStop(to_station));

        optional<Router<double>::RouteInfo> result_route = router_->BuildRoute(from_vid, to_vid);
        if (!result_route.has_value()) {
            return nullopt;
        }

        FoundRouteResult result = {result_route->weight, {}};
        FoundRouteResult::Wait wait_on_entering_station{string(from_station), static_cast<double>(router_settings_.bus_wait_time)};
        result.route.emplace_back(move(wait_on_entering_station));

        for (const EdgeId edge_id : result_route->edges) {
            if (buses_.at(edge_id).has_value()) { // поездка
                Stop* from_stop = stops_[graph_->GetEdge(edge_id).from];
                Stop* to_stop = stops_[graph_->GetEdge(edge_id).to];
                Bus* bus = *buses_.at(edge_id);
                size_t span_count = 0;
                bool found_from_stop = false;
                bool found_to_stop = false;

                for (Stop* stop : bus->stops) {
                    if (from_stop == stop) {found_from_stop = true;}
                    if (to_stop == stop) {found_to_stop = true;}
                    if ((found_from_stop && !found_to_stop) || (!found_from_stop && found_to_stop)) {
                        ++span_count;
                    }
                    if (found_from_stop && found_to_stop) {
                        break;
                    }
                }

                FoundRouteResult::Bus bus_result{span_count, bus->name, graph_->GetEdge(edge_id).weight};
                result.route.emplace_back(move(bus_result));
            } else { // пересадка
                Stop* from_stop = stops_[graph_->GetEdge(edge_id).from];
                FoundRouteResult::Wait wait_result{from_stop->name, graph_->GetEdge(edge_id).weight};
                result.route.emplace_back(move(wait_result));
            }
        }
        if (holds_alternative<FoundRouteResult::Wait>(result.route.back())) {
            result.route.pop_back();
        }
        return result;
    }
} // namespace transport_router