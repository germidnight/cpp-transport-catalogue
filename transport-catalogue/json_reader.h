#pragma once
/*
 * Код наполнения транспортного справочника данными из JSON, а также код обработки запросов к базе и формирование массива ответов в формате JSON.
 * Формат входных данных, данные поступают из stdin в формате JSON-объекта. Его верхнеуровневая структура (в любом порядке):
 * {
 *   "base_requests": [ ... ],
 *   "render_settings": { ... },
 *   "stat_requests": [ ... ]
 * }
 * Это словарь, содержащий ключи:
 * 1) base_requests — массив с описанием автобусных маршрутов и остановок,
 * 2) stat_requests — массив с запросами к транспортному справочнику (см. request_handler.cpp),
 * 3) render_settings - словарь с настройками отрисовки транспортного каталога
 *
 * Массив base_requests содержит элементы двух типов: маршруты и остановки. Они перечисляются в произвольном порядке.
 * 1) Описание остановки:
 * {
 *   "type": "Stop",
 *   "name": "Имя остановки",
 *   "latitude": 43.598701,
 *   "longitude": 39.730623,
 *   "road_distances": {
 *     "Имя остановки 1": 3000,
 *     "Имя остановки 2": 4300
 *   }
 * }
 * - type — строка, равная "Stop". Означает, что словарь описывает остановку;
 * - name — название остановки;
 * - latitude и longitude — широта и долгота остановки — числа с плавающей запятой;
 * - road_distances — словарь, задающий дорожное расстояние от этой остановки до соседних.
 * Каждый ключ в этом словаре — название соседней остановки, значение — целочисленное расстояние в метрах.
 *
 * 2) Пример описания автобусного маршрута:
 * {
 *   "type": "Bus",
 *   "name": "Номер автобуса",
 *   "stops": [
 *     "Имя остановки 1",
 *     "Имя остановки 2",
 *     "Имя остановки 3",
 *     "Имя остановки 1"
 *   ],
 *   "is_roundtrip": true
 * }
 * Описание автобусного маршрута — словарь с ключами:
 * - type — строка "Bus". Означает, что словарь описывает автобусный маршрут;
 * - name — название маршрута;
 * - stops — массив с названиями остановок, через которые проходит маршрут.
 * У кольцевого маршрута название последней остановки дублирует название первой. Например: ["stop1", "stop2", "stop3", "stop1"];
 * - is_roundtrip — значение типа bool. true, если маршрут кольцевой.
 *
 * 3) Структура словаря render_settings:
 * - width и height — ширина и высота изображения в пикселях. Вещественное число в диапазоне от 0 до 100000.
 * - padding — отступ краёв карты от границ SVG-документа. Вещественное число не меньше 0 и меньше min(width, height)/2.
 * - line_width — толщина линий, которыми рисуются автобусные маршруты. Вещественное число в диапазоне от 0 до 100000.
 * - stop_radius — радиус окружностей, которыми обозначаются остановки. Вещественное число в диапазоне от 0 до 100000.
 * - bus_label_font_size — размер текста, которым написаны названия автобусных маршрутов. Целое число в диапазоне от 0 до 100000.
 * - bus_label_offset — смещение надписи с названием маршрута относительно координат конечной остановки на карте.
 * Массив из двух элементов типа double. Задаёт значения свойств dx и dy SVG-элемента <text>. Элементы массива — числа в диапазоне от –100000 до 100000.
 * - stop_label_font_size — размер текста, которым отображаются названия остановок. Целое число в диапазоне от 0 до 100000.
 * - stop_label_offset — смещение названия остановки относительно её координат на карте.
 * Массив из двух элементов типа double. Задаёт значения свойств dx и dy SVG-элемента <text>. Числа в диапазоне от –100000 до 100000.
 * - underlayer_color — цвет подложки под названиями остановок и маршрутов.
 * - underlayer_width — толщина подложки под названиями остановок и маршрутов. Задаёт значение атрибута stroke-width элемента <text>.
 * Вещественное число в диапазоне от 0 до 100000.
 * - color_palette — цветовая палитра, не пустой массив цветов.
 *
 * Цвет можно указать в одном из следующих форматов:
 * - в виде строки, например, "red" или "black";
 * - в массиве из трёх целых чисел диапазона [0, 255]. Они определяют r, g и b компоненты цвета
 * в формате svg::Rgb. Цвет [255, 16, 12] нужно вывести в SVG как rgb(255,16,12);
 * - в массиве из четырёх элементов: три целых числа в диапазоне от [0, 255] и одно вещественное число
 * в диапазоне от [0.0, 1.0]. Они задают составляющие red, green, blue и opacity цвета формата svg::Rgba.
 * Цвет, заданный как [255, 200, 23, 0.85], должен быть выведен в SVG как rgba(255,200,23,0.85).
 */
#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

#include <map>
#include <string>
#include <utility>
#include <vector>

namespace transport {
    namespace json_reader {
        struct StatRequest {
            StatRequest(int val_id, std::string val_type, std::string val_name)
                        : id(val_id), type(val_type), name(val_name) {}
            int id = 0;
            std::string type;
            std::string name;
        };

        class JsonReader {
        public:
            /*
             * Наполняет данными транспортный справочник, из json::Document в три прохода:
             * 1) пробегает весь JSON документ:
             *          - вносит в каталог записи "Stop" без расстояний;
             *          - ссылки на ноды с массивами расстояний до остановок складываем в вектор stop_distances_;
             *          - ссылки на ноды с записями маршрутов в вектор buses_;
             *          - ссылки на ноды с запросами в вектор requests_;
             * 2) по вектору stop_distances_ вносит в каталог расстояния до остановок;
             * 3) по вектору buses_ вносит в каталог информацию о маршрутах
             * Возвращает вектор с ссылками на ноды с запросами
             */
            void FillTransportCatalogue(json::Document &document, catalogue::TransportCatalogue &catalogue);

            /* --------------------- настройки складываем в структуру map_render_settings_, она пойдёт в map_renderer.cpp ---------------------- */
            map_renderer::RenderSettings FillRenderSettings(const json::Document &document);

            /* --------------------- запросы складываем в вектор requests_, он пойдёт в request+handler.cpp ---------------------- */
            const std::vector<StatRequest>& FillStatRequests(const json::Document &document);

        private:
            /* --------------------- Обрабатываем запросы на пополнение базы ---------------------- */
            void ReadBaseRequests(const json::Node &node, catalogue::TransportCatalogue &catalogue);    // первый проход
            void FillStopDistances(catalogue::TransportCatalogue &catalogue);                           // второй проход
            void FillBusses(catalogue::TransportCatalogue &catalogue);                                  // третий проход

            svg::Color WhatColor(const json::Node& clr_node);

            std::vector < std::pair<std::string, const json::Node*>> stop_distances_;
            std::vector<const json::Node*> buses_;
            std::vector<StatRequest> requests_;

            const std::string request_type_fill_ = "base_requests";
            const std::string request_type_stat_ = "stat_requests";

            const std::string type_ = "type";
            const std::string name_ = "name";
            const std::string id_ = "id";

            const std::string stop_type_ = "Stop";
            const std::string stop_lat_ = "latitude";
            const std::string stop_long_ = "longitude";
            const std::string stop_road_dist_ = "road_distances";

            const std::string bus_type_ = "Bus";
            const std::string bus_stops_ = "stops";
            const std::string bus_roundtrip_ = "is_roundtrip";

            const std::string str_render_settings_ = "render_settings";
            const std::string str_width_ = "width";
            const std::string str_height_ = "height";
            const std::string str_padding_ = "padding";
            const std::string str_line_width_ = "line_width";
            const std::string str_stop_radius_ = "stop_radius";
            const std::string str_bus_label_font_size_ = "bus_label_font_size";
            const std::string str_bus_label_offset_ = "bus_label_offset";
            const std::string str_stop_label_font_size_ = "stop_label_font_size";
            const std::string str_stop_label_offset_ = "stop_label_offset";
            const std::string str_underlayer_color_ = "underlayer_color";
            const std::string str_underlayer_width_ = "underlayer_width";
            const std::string str_color_palette_ = "color_palette";
        };

    } // namespace json_reader

} // namespace transport