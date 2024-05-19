/*
 * Код обработчика запросов к базе
 */
#include "request_handler.h"

#include <optional>
#include <sstream>

namespace transport {
    namespace request_handler {

       json::Document RequestHandler::GetStatistics() {
            using namespace catalogue;
            using namespace json;
            using namespace json_reader;

            Array answer_arr;
            answer_arr.reserve(requests_.size());

            for(const StatRequest& req : requests_) {
                if(req.type == bus_type_) {
                    BusStatRequest(req, answer_arr);

                } else if(req.type == stop_type_) {
                    StopStatRequest(req, answer_arr);

                } else if(req.type == map_type_) {
                    MapStatRequest(req, answer_arr);
                }
            }
            return Document(answer_arr);
        }

        void RequestHandler::BusStatRequest(const json_reader::StatRequest &req, json::Array &answer_arr) {
            using namespace catalogue;
            using namespace json;
            using namespace json_reader;

            std::optional<BusStatistics> bus_info = catalogue_.GetBusStatistics(req.name);
            if (bus_info.has_value()) {
                answer_arr.emplace_back(Dict{
                    {bus_curvature_, static_cast<double>(bus_info->curvature)},
                    {request_id_, req.id},
                    {bus_route_, static_cast<double>(bus_info->distance)},
                    {bus_stop_cnt_, static_cast<int>(bus_info->stops_num)},
                    {bus_stop_unique_, static_cast<int>(bus_info->uniq_stops_num)},
                });
            } else {
                answer_arr.emplace_back(Dict{
                    {request_id_, req.id},
                    {error_, error_string_},
                });
            }
        }

        void RequestHandler::StopStatRequest(const json_reader::StatRequest &req, json::Array &answer_arr) {
            using namespace catalogue;
            using namespace json;
            using namespace json_reader;

            std::optional<std::vector<std::string>> stop_info = catalogue_.GetStopStatistics(req.name);

            if (!stop_info.has_value()) {
                answer_arr.emplace_back(Dict{
                    {request_id_, req.id},
                    {error_, error_string_},
                });
            } else if (stop_info.value().empty()) {
                Array buses_empty;
                answer_arr.emplace_back(Dict{
                    {stop_buses, buses_empty},
                    {request_id_, req.id}});
            } else {
                Array buses;
                for (const std::string &bus : stop_info.value()) {
                    buses.emplace_back(bus);
                }
                answer_arr.emplace_back(Dict{
                    {stop_buses, buses},
                    {request_id_, req.id}});
            }
        }

        void RequestHandler::MapStatRequest(const json_reader::StatRequest &req, json::Array &answer_arr) {
            using namespace catalogue;
            using namespace json;

            // Формируем карту
            map_renderer::MapRenderer map_drawer(catalogue_, draw_settings_);
            svg::Document &map_picture = map_drawer.DrawMap();
            std::ostringstream out;
            map_picture.Render(out);

            answer_arr.emplace_back(Dict{
                    {map_answer, out.str()},
                    {request_id_, req.id},
                });
        }

    } // namespace request_handler

} // namespace transport