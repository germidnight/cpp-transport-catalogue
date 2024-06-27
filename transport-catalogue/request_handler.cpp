/*
 * Код обработчика запросов к базе
 */
#include "request_handler.h"

#include <optional>
#include <sstream>
#include <variant>

namespace transport {
    namespace request_handler {

       json::Document RequestHandler::GetStatistics() {
            using namespace catalogue;
            using namespace json;
            using namespace json_reader;
            using namespace transport_router;

            Builder answer_arr;
            answer_arr.StartArray();

            for(const StatRequest& req : requests_) {
                if (req.type == str_bus_type_) {
                    BusStatRequest(req, answer_arr);
                } else if (req.type == str_stop_type_) {
                    StopStatRequest(req, answer_arr);
                } else if (req.type == str_map_type_) {
                    MapStatRequest(req, answer_arr);
                } else if (req.type == str_route_) {
                    if (route_builder_ == nullptr) {
                        route_builder_ = new RouteBuilder(catalogue_, router_settings_);
                    }
                    RouteStatRequest(req, answer_arr);
                }
            }
            answer_arr.EndArray();

            if (route_builder_ != nullptr) {
                delete route_builder_;
            }

            return Document(answer_arr.Build());
        }

        void RequestHandler::BusStatRequest(const json_reader::StatRequest &req, json::Builder &answer_arr) {
            using namespace catalogue;
            using namespace json;
            using namespace json_reader;

            std::optional<BusStatistics> bus_info = catalogue_.GetBusStatistics(req.name);

            answer_arr.StartDict();
            if (bus_info.has_value()) {
                answer_arr.Key(str_bus_curvature_).Value(static_cast<double>(bus_info->curvature))
                        .Key(str_request_id_).Value(req.id)
                        .Key(str_bus_route_).Value(static_cast<double>(bus_info->distance))
                        .Key(str_bus_stop_cnt_).Value(static_cast<int>(bus_info->stops_num))
                        .Key(str_bus_stop_unique_).Value(static_cast<int>(bus_info->uniq_stops_num));
            } else {
                answer_arr.Key(str_request_id_).Value(req.id)
                        .Key(str_error_).Value(str_error_string_);
            }
            answer_arr.EndDict();
        }

        void RequestHandler::StopStatRequest(const json_reader::StatRequest &req, json::Builder &answer_arr) {
            using namespace catalogue;
            using namespace json;
            using namespace json_reader;

            std::optional<std::vector<std::string>> stop_info = catalogue_.GetStopStatistics(req.name);

            answer_arr.StartDict();
            if (!stop_info.has_value()) {
                answer_arr.Key(str_request_id_).Value(req.id)
                        .Key(str_error_).Value(str_error_string_);
            } else if (stop_info.value().empty()) {
                answer_arr.Key(str_stop_buses_).StartArray().EndArray()
                        .Key(str_request_id_).Value(req.id);
            } else {
                answer_arr.Key(str_stop_buses_).StartArray();

                for (const std::string &bus : stop_info.value()) {
                    answer_arr.Value(bus);
                }

                answer_arr.EndArray()
                            .Key(str_request_id_).Value(req.id);
            }
            answer_arr.EndDict();
        }

        void RequestHandler::MapStatRequest(const json_reader::StatRequest &req, json::Builder &answer_arr) {
            using namespace catalogue;
            using namespace json;
            using namespace transport_router;

            map_renderer::MapRenderer map_drawer(catalogue_, draw_settings_);
            svg::Document &map_picture = map_drawer.DrawMap();
            std::ostringstream out;
            map_picture.Render(out);

            answer_arr.StartDict()
                        .Key(str_map_answer_).Value(out.str())
                        .Key(str_request_id_).Value(req.id)
                    .EndDict();
        }

        void RequestHandler::RouteStatRequest(const json_reader::StatRequest &req, json::Builder &answer_arr) {
            using namespace catalogue;
            using namespace json;
            using namespace transport_router;

            std::optional<FoundRouteResult> found_route = route_builder_->FindRoute(req.from, req.to);

            answer_arr.StartDict();
            if (!found_route.has_value()) {
                answer_arr.Key(str_request_id_).Value(req.id)
                            .Key(str_error_).Value(str_error_string_);
            } else {
                answer_arr.Key(str_request_id_).Value(req.id)
                            .Key(str_total_time_).Value(found_route->total_time)
                            .Key(str_items_).StartArray();
                for (auto& item : found_route->route) {
                    answer_arr.StartDict();
                    if (std::holds_alternative<FoundRouteResult::Wait>(item)) {
                        // wait
                        FoundRouteResult::Wait wait = std::get<FoundRouteResult::Wait>(item);
                        answer_arr.Key(str_type_).Value(str_type_wait_)
                                    .Key(str_stop_name_).Value(wait.stop)
                                    .Key(str_time_).Value(wait.time);
                    } else {
                        // bus
                        FoundRouteResult::Bus bus = std::get<FoundRouteResult::Bus>(item);
                        answer_arr.Key(str_type_).Value(str_type_bus_)
                                    .Key(str_bus_).Value(bus.bus)
                                    .Key(str_span_count_).Value(static_cast<int>(bus.span_count))
                                    .Key(str_time_).Value(bus.time);
                    }
                    answer_arr.EndDict();
                }
                answer_arr.EndArray();
            }
            answer_arr.EndDict();
        }
    } // namespace request_handler

} // namespace transport