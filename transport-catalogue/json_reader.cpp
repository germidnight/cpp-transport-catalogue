/*
 * код наполнения транспортного справочника данными из JSON, а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */
#include "json_reader.h"

using namespace json;

namespace transport {

    using namespace catalogue;

    namespace json_reader {

        void JsonReader::FillTransportCatalogue(Document &document, TransportCatalogue &catalogue) {
            const Node &root_node = document.GetRoot();
            try {
/* --------------------- Обрабатываем запросы на пополнение базы ---------------------- */
                ReadBaseRequests(root_node, catalogue);
                FillStopDistances(catalogue);
                FillBusses(catalogue);

            } catch (const std::out_of_range &e) {
                e.what(); // No such element
                throw e;
            } catch (const std::logic_error &e) {
                e.what(); // Not a map, array, string, double, bool
                throw e;
            } catch (const std::exception &e) {
                e.what(); // Другая ошибка
                throw e;
            }
        }
        /*
         * - вносит в каталог записи "Stop" без расстояний;
         * - указатели на ноды с массивами расстояний до остановок складывает в вектор stop_distances_;
         * - указатели на ноды с записями маршрутов складывает в вектор buses_
         */
        void JsonReader::ReadBaseRequests(const Node &node, TransportCatalogue &catalogue) {
            try {
                const Array &base_fill_array = node.AsMap().at(request_type_fill_).AsArray();
                for (const Node &fill_node : base_fill_array) {
                    if(fill_node.AsMap().at(type_).AsString() == stop_type_) { // обработка записей с остановками
                        catalogue.AddStop(fill_node.AsMap().at(name_).AsString(),
                                          {fill_node.AsMap().at(stop_lat_).AsDouble(), fill_node.AsMap().at(stop_long_).AsDouble()});
                        if (fill_node.AsMap().count(stop_road_dist_)) {
                            stop_distances_.emplace_back(std::make_pair(fill_node.AsMap().at(name_).AsString(),
                                                                        std::addressof(fill_node.AsMap().at(stop_road_dist_))));
                        }
                    } else if (fill_node.AsMap().at(type_).AsString() == bus_type_) { // сохранение ссылок на записи с маршрутами
                        buses_.emplace_back(std::addressof(fill_node));
                    }
                }
            } catch (const std::out_of_range &e) {
                e.what(); // No such element
                throw e;
            } catch (const std::logic_error &e) {
                e.what(); // Not a map, array, string, double
                throw e;
            } catch (const std::exception &e) {
                e.what(); // Другая ошибка
                throw e;
            }
        }

        void JsonReader::FillStopDistances(TransportCatalogue& catalogue) {
            for(const auto& [stop_from, road_dist_node] : stop_distances_) {
                try {
                    for(const auto& [stop_to, node_dist] : road_dist_node->AsMap()) {
                        catalogue.AddStopDistances(stop_from, stop_to, static_cast<size_t>(node_dist.AsInt()));
                    }
                } catch (const std::out_of_range &e) {
                    e.what(); // No such element
                    throw e;
                } catch (const std::logic_error &e) {
                    e.what(); // Not a map, int
                    throw e;
                }
            }
        }

        void JsonReader::FillBusses(TransportCatalogue &catalogue) {
            for(size_t i = 0; i != buses_.size(); ++i) {
                try {
                    bool is_roundtrip = true;
                    std::vector<std::string_view> stop_names;
                    for (const Node &node_stop : buses_[i]->AsMap().at(bus_stops_).AsArray()) {
                        stop_names.emplace_back(node_stop.AsString());
                    }
                    if (!buses_[i]->AsMap().at(bus_roundtrip_).AsBool()) { // нужно замкнуть маршрут
                        is_roundtrip = false;
                        stop_names.insert(stop_names.end(), std::next(stop_names.rbegin()), stop_names.rend());
                    }
                    catalogue.AddBus(buses_[i]->AsMap().at(name_).AsString(), stop_names, is_roundtrip);
                } catch (const std::out_of_range &e) {
                    e.what(); // No such element
                    throw e;
                } catch (const std::logic_error &e) {
                    e.what(); // Not a map, array, string, bool
                    throw e;
                }
            }
        }

        const std::vector<StatRequest>& JsonReader::FillStatRequests(const json::Document &document) {
            const Node &node = document.GetRoot();
            try {
                const Array &requests_array = node.AsMap().at(request_type_stat_).AsArray();
                for (const Node &req_node : requests_array) {
                    std::string name_string;
                    if (req_node.AsMap().count(name_)) {
                        name_string = req_node.AsMap().at(name_).AsString();
                    }
                    requests_.emplace_back(StatRequest(req_node.AsMap().at(id_).AsInt()
                                        , req_node.AsMap().at(type_).AsString()
                                        , name_string));
                }
            } catch (const std::out_of_range &e) {
                e.what(); // No such element
                throw e;
            } catch (const std::logic_error &e) {
                e.what(); // Not an array, map, string, int
                throw e;
            } catch (const std::exception &e) {
                e.what(); // Другая ошибка
                throw e;
            }
            return requests_;
        }

        svg::Color JsonReader::WhatColor(const Node& clr_node) {
            if (clr_node.IsString()) {
                return clr_node.AsString();
            } else if (clr_node.IsArray()) {
                if (clr_node.AsArray().size() == 4) {
                    const svg::Rgba rgba_clr(static_cast<uint8_t>(clr_node.AsArray().at(0).AsInt()),
                                             static_cast<uint8_t>(clr_node.AsArray().at(1).AsInt()),
                                             static_cast<uint8_t>(clr_node.AsArray().at(2).AsInt()),
                                             (clr_node.AsArray().at(3).AsDouble()));
                    return rgba_clr;
                } else if(clr_node.AsArray().size() == 3) {
                    const svg::Rgb rgb_clr(static_cast<uint8_t>(clr_node.AsArray().at(0).AsInt()),
                                           static_cast<uint8_t>(clr_node.AsArray().at(1).AsInt()),
                                           static_cast<uint8_t>(clr_node.AsArray().at(2).AsInt()));
                    return rgb_clr;
                }
            }
            return std::monostate{};
        }

        map_renderer::RenderSettings JsonReader::FillRenderSettings(const json::Document &document) {
            const Node &node = document.GetRoot();
            map_renderer::RenderSettings map_render_settings;
            try {
                const Dict &settings_dict = node.AsMap().at(str_render_settings_).AsMap();
                if (settings_dict.count(str_width_)) {
                    map_render_settings.width = settings_dict.at(str_width_).AsDouble();
                }
                if (settings_dict.count(str_height_)) {
                    map_render_settings.height = settings_dict.at(str_height_).AsDouble();
                }
                if (settings_dict.count(str_padding_)) {
                    map_render_settings.padding = settings_dict.at(str_padding_).AsDouble();
                }
                if (settings_dict.count(str_line_width_)) {
                    map_render_settings.line_width = settings_dict.at(str_line_width_).AsDouble();
                }
                if (settings_dict.count(str_stop_radius_)) {
                    map_render_settings.stop_radius = settings_dict.at(str_stop_radius_).AsDouble();
                }
                if (settings_dict.count(str_bus_label_font_size_)) {
                    map_render_settings.bus_label_font_size = settings_dict.at(str_bus_label_font_size_).AsInt();
                }
                if (settings_dict.count(str_bus_label_offset_)) {
                    map_render_settings.bus_label_offset.x = settings_dict.at(str_bus_label_offset_).AsArray().at(0).AsDouble();
                    map_render_settings.bus_label_offset.y = settings_dict.at(str_bus_label_offset_).AsArray().at(1).AsDouble();
                }
                if (settings_dict.count(str_stop_label_font_size_)) {
                    map_render_settings.stop_label_font_size = settings_dict.at(str_stop_label_font_size_).AsInt();
                }
                if (settings_dict.count(str_stop_label_offset_)) {
                    map_render_settings.stop_label_offset.x = settings_dict.at(str_stop_label_offset_).AsArray().at(0).AsDouble();
                    map_render_settings.stop_label_offset.y = settings_dict.at(str_stop_label_offset_).AsArray().at(1).AsDouble();
                }
                if (settings_dict.count(str_underlayer_color_)) {
                    map_render_settings.underlayer_color = WhatColor(settings_dict.at(str_underlayer_color_));
                }
                if (settings_dict.count(str_underlayer_width_)) {
                    if (settings_dict.at(str_underlayer_width_).IsDouble()) {
                        map_render_settings.underlayer_width = settings_dict.at(str_underlayer_width_).AsDouble();
                    } else {
                        map_render_settings.underlayer_width = static_cast<double>(settings_dict.at(str_underlayer_width_).AsInt());
                    }
                }
                if (settings_dict.count(str_color_palette_)) {
                    for(const Node& color_node : settings_dict.at(str_color_palette_).AsArray()) {
                        map_render_settings.color_palette.emplace_back(WhatColor(color_node));
                    }
                }
            } catch (const std::out_of_range &e) {
                e.what(); // No such element
                throw e;
            } catch (const std::logic_error &e) {
                e.what(); // Not an array, map, double, int, string
                throw e;
            } catch (const std::exception &e) {
                e.what(); // Другая ошибка
                throw e;
            }
            return map_render_settings;
        }

    } // namespace json_reader

} // namespace transport