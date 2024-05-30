#pragma once
/*
 * Код обработчика запросов к базе.
 *
 * Запросы хранятся в массиве stat_requests. В ответ на них программа должна вывести в stdout JSON-массив ответов:
 * [
 *   { ответ на первый запрос },
 *   { ответ на второй запрос },
 *   ...
 *   { ответ на последний запрос }
 * ]
 * Каждый запрос — словарь с обязательными ключами id и type. Они задают уникальный числовой идентификатор запроса и его тип.
 * В словаре могут быть и другие ключи, специфичные для конкретного типа запроса.
 * В выходном JSON-массиве на каждый запрос stat_requests должен быть ответ в виде словаря с обязательным ключом request_id.
 * Значение ключа должно быть равно id соответствующего запроса. В словаре возможны и другие ключи, специфичные для конкретного типа ответа.
 * Порядок следования ответов на запросы в выходном массиве должен совпадать с порядком запросов в массиве stat_requests.
 *
 * 1) Получение информации о маршруте. Формат запроса:
 * {
 *   "id": 12345678,
 *   "type": "Bus",
 *   "name": "14"
 * }
 * - Ключ id - целое число (идентификатор запроса)
 * - Ключ type имеет значение “Bus”. По нему можно определить, что это запрос на получение информации о маршруте.
 * - Ключ name задаёт название маршрута, для которого приложение должно вывести статистическую информацию.
 * Ответ на этот запрос должен быть дан в виде словаря:
 * {
 *   "curvature": 2.18604,
 *   "request_id": 12345678,
 *   "route_length": 9300,
 *   "stop_count": 4,
 *   "unique_stop_count": 3
 * }
 * Ключи словаря:
 * - curvature — извилистость маршрута. Она равна отношению длины дорожного расстояния маршрута к длине географического расстояния. Число типа double;
 * - request_id — должен быть равен id соответствующего запроса Bus. Целое число;
 * - route_length — длина дорожного расстояния маршрута в метрах, целое число;
 * - stop_count — количество остановок на маршруте;
 * - unique_stop_count — количество уникальных остановок на маршруте.
 * Если в справочнике нет маршрута с указанным названием, ответ должен быть таким:
 * {
 *   "request_id": 12345678,
 *   "error_message": "not found"
 * }
 *
 * 2) Получение информации об остановке. Формат запроса:
 * {
 *   "id": 12345,
 *   "type": "Stop",
 *   "name": "Улица Докучаева"
 * }
 * - Ключ type имеет значение “Stop”. По нему можно определить, что это запрос на получение информации об остановке.
 * - Ключ name задаёт название остановки.
 * Ответ на запрос:
 * {
 *   "buses": [
 *       "14", "22к"
 *   ],
 *   "request_id": 12345
 * }
 * Значение ключей ответа:
 * - buses — массив названий маршрутов, которые проходят через эту остановку. Названия отсортированы в лексикографическом порядке.
 * - request_id — целое число, равное id соответствующего запроса Stop.
 * Если в справочнике нет остановки с переданным названием, ответ на запрос должен быть такой:
 * {
 *   "request_id": 12345,
 *   "error_message": "not found"
 * }
 *
 * 3) Запрос на получение изображения. Формат запроса:
 * {
 *   "type": "Map",
 *   "id": 11111
 * }
 * Ответ на запрос:
 * {
 *   "map": "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n  <polyline points=\"100.817,170 30,30 100.817,170\" fill=\"none\" stroke=\"green\" stroke-width=\"14\" stroke-linecap=\"round\" stroke-linejoin=\"round\"/>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"100.817\" y=\"170\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <text fill=\"green\" x=\"100.817\" y=\"170\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"30\" y=\"30\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <text fill=\"green\" x=\"30\" y=\"30\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <circle cx=\"100.817\" cy=\"170\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"30\" cy=\"30\" r=\"5\" fill=\"white\"/>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"100.817\" y=\"170\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Морской вокзал</text>\n  <text fill=\"black\" x=\"100.817\" y=\"170\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Морской вокзал</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"30\" y=\"30\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Ривьерский мост</text>\n  <text fill=\"black\" x=\"30\" y=\"30\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Ривьерский мост</text>\n</svg>",
 *   "request_id": 11111
 * }
 */

#include "json.h"
#include "json_builder.h"
#include "json_reader.h"
#include "transport_catalogue.h"

#include <string>

namespace transport {
    namespace request_handler {

        class RequestHandler {
        public:
            RequestHandler(const std::vector<json_reader::StatRequest> requests,
                            const catalogue::TransportCatalogue &catalogue,
                            const map_renderer::RenderSettings &settings)
                : requests_(requests), catalogue_(catalogue), draw_settings_(settings) {}

            json::Document GetStatistics();

        private:
            void BusStatRequest(const json_reader::StatRequest &req, json::Builder &answer_arr);
            void StopStatRequest(const json_reader::StatRequest &req, json::Builder &answer_arr);
            void MapStatRequest(const json_reader::StatRequest &req, json::Builder &answer_arr);

            const std::vector<json_reader::StatRequest> requests_;
            const catalogue::TransportCatalogue &catalogue_;
            const map_renderer::RenderSettings &draw_settings_;

            const std::string request_id_ = "request_id";
            const std::string type_ = "type";
            const std::string name_ = "name";
            const std::string error_ = "error_message";
            const std::string error_string_ = "not found";

            const std::string stop_type_ = "Stop";
            const std::string stop_buses_ = "buses";

            const std::string bus_type_ = "Bus";
            const std::string bus_curvature_ = "curvature";
            const std::string bus_route_ = "route_length";
            const std::string bus_stop_cnt_ = "stop_count";
            const std::string bus_stop_unique_ = "unique_stop_count";

            const std::string map_type_ = "Map";
            const std::string map_answer_ = "map";
        };

    } // namespace request_handler

} // namespace transport