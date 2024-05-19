/*
 * Карта состоит из четырёх типов объектов. Порядок их вывода в SVG-документ:
 * 1) ломаные линии маршрутов,
 * 2) названия маршрутов,
 * 3) круги, обозначающие остановки,
 * 4) названия остановок.
 *
 * Линии маршрутов должны выводиться в виде ломаной линии, соединяющей остановки (круги).
 * Порядок рисования: по возрастанию названий маршрутов. Линии маршрутов, на которых нет остановок, рисоваться не должны.
 * Свойства линий:
 * 1) цвет линии stroke определён по правилам:
 * Из палитры color_palette выбираются цвета линий в соответствии с лексикографическим порядком названий маршрутов:
 * - первый по алфавиту маршрут должен получить первый цвет, второй маршрут — второй цвет и так далее.
 * - Если маршрутов больше, чем цветов в палитре, цвета переиспользуются по циклу.
 * - Если на маршруте нет остановок, следующий за ним маршрут должен использовать в палитре тот же индекс.
 * 2) цвет заливки fill должен иметь значение none;
 * 3) толщина линии stroke-width равна настройке line_width;
 * 4) Формы конца линии stroke-linecap и соединений stroke-linejoin равны round.
 *
 * Вершины каждой ломаной — это координаты соответствующих остановок. Выведите их в порядке следования от первой до первой остановки по кольцевому маршруту.
 * Для проецирования координат остановок на SVG-карту используйте класс SphereProjector. Количество вершин должно быть равно величине stop_count из ответа на запрос Bus.
 * Если маршрут некольцевой, то есть "is_roundtrip": false, каждый отрезок между соседними остановками должен быть нарисован дважды: сначала в прямом, а потом в обратном направлении.
 *
 * Название маршрута должно отрисовываться у каждой из его конечных остановок.
 * В кольцевом маршруте — когда "is_roundtrip": true — конечной считается первая остановка маршрута.
 * А в некольцевом — первая и последняя.
 * Названия маршрутов должны быть нарисованы в алфавитном порядке.
 * Для каждого маршрута сначала выводится название для его первой конечной остановки, а затем, если маршрут некольцевой
 * и конечные не совпадают, — для второй конечной. Если остановок у маршрута нет, его название выводиться не должно.
 * Для каждого названия у конечной выведите два текстовых объекта: подложку и саму надпись.
 * Общие свойства обоих объектов:
 * 1) x и y — координаты соответствующей остановки;
 * 2) смещение dx и dy равно настройке bus_label_offset;
 * 3) размер шрифта font-size равен настройке bus_label_font_size;
 * 4) название шрифта font-family — "Verdana";
 * 5) толщина шрифта font-weight — "bold";
 * 6) содержимое — название автобуса.
 * Дополнительные свойства подложки:
 * 1) цвет заливки fill и цвет линий stroke равны настройке underlayer_color;
 * 2) толщина линий stroke-width равна настройке underlayer_width;
 * 3) формы конца линии stroke-linecap и соединений stroke-linejoin равны round.
 * Дополнительное свойство самой надписи - цвет заливки fill равен цвету соответствующего автобусного маршрута из палитры.
 *
 * Символы остановок. Каждая остановка маршрута изображается на карте в виде кружочков белого цвета.
 * Их нужно отрисовать для всех маршрутов, где есть остановки. Одна остановка — один кружочек.
 * Выводите их по одному в порядке возрастания названия остановки, независимо от количества маршрутов, которые через неё проходят.
 * Остановки, через которые не проходит ни один автобус, выводиться не должны.
 * Атрибуты окружности:
 * 1) координаты центра cx и cy — координаты соответствующей остановки на карте;
 * 2) радиус r равен настройке stop_radius из словаря render_settings;
 * 3) цвет заливки fill — "white".
 *
 * Названия остановок. Названия остановок выводятся в лексикографическом порядке, по одному на каждую остановку,
 * независимо от количества автобусов, которые через неё проходят.
 * Названия остановок, через которые не проходит ни один маршрут, отрисовывать не надо.
 * Для каждой остановки выведите два текстовых объекта: подложку и саму надпись.
 * Общие свойства обоих объектов:
 * 1) x и y — координаты соответствующей остановки;
 * 2) смещение dx и dy равно настройке stop_label_offset;
 * 3) размер шрифта font-size равен настройке stop_label_font_size;
 * 4) название шрифта font-family — "Verdana";
 * 5) свойства font-weight быть не должно;
 * 6) содержимое — название остановки.
 * Дополнительные свойства подложки:
 * 1) цвет заливки fill и цвет линий stroke равны настройке underlayer_color;
 * 2) толщина линий stroke-width равна настройке underlayer_width;
 * 3) формы конца линии stroke-linecap и соединений stroke-linejoin равны "round".
 * Дополнительное свойство самой надписи - цвет заливки fill — "black".
 */

#include "domain.h"
#include "map_renderer.h"

#include <algorithm>
#include <utility>

namespace transport {
    namespace map_renderer {

        svg::Document &MapRenderer::DrawMap() {
            using namespace catalogue;

            all_stop_names_ = catalogue_.GetAllStopNames();
            std::sort(all_stop_names_.begin(), all_stop_names_.end());

            std::vector<geo::Coordinates> all_coords;
            all_coords.reserve(all_stop_names_.size());

            for (auto it = all_stop_names_.begin(); it != all_stop_names_.end(); ++it) {
                const Stop* stop = catalogue_.FindStop(*it);
                if (stop != nullptr) {
                    all_coords.emplace_back(stop->location);
                }
            }
            const geo::SphereProjector projector{
                        all_coords.begin(), all_coords.end(),
                        draw_settings_.width, draw_settings_.height, draw_settings_.padding
                    };

            all_bus_names_ = catalogue_.GetAllBusNames();
            std::sort(all_bus_names_.begin(), all_bus_names_.end());

            DrawBusLines(projector);
            DrawBusNames(projector);
            DrawStopCircles(projector);
            DrawStopNames(projector);

            return map_picture_;
        }

        void MapRenderer::DrawBusLines(const geo::SphereProjector &projector) {
            using namespace transport::catalogue;
            using namespace std::literals;

            size_t color_idx = 0;
            const size_t palette_color_last = draw_settings_.color_palette.size() - 1;

            for (auto it = all_bus_names_.begin(); it != all_bus_names_.end(); ++it) {
                const Bus *bus = catalogue_.FindBus(*it);
                if (!bus->stops.empty()) { // Линии маршрутов, на которых нет остановок, рисоваться не должны.
                    svg::Polyline polyline;
                    polyline.SetStrokeColor(draw_settings_.color_palette.at(color_idx));
                    polyline.SetFillColor("none"s);
                    polyline.SetStrokeWidth(draw_settings_.line_width);
                    polyline.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
                    polyline.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

                    for (const Stop *stop : bus->stops) {
                        polyline.AddPoint(projector(stop->location));
                    }
                    map_picture_.Add(polyline);

                    color_idx = (color_idx < palette_color_last) ? color_idx + 1 : 0;
                }
            }
        }

        void MapRenderer::DrawBusNames(const geo::SphereProjector &projector) {
            using namespace transport::catalogue;
            using namespace std::literals;

            size_t color_idx = 0;
            const size_t palette_color_last = draw_settings_.color_palette.size() - 1;

            for (auto it = all_bus_names_.begin(); it != all_bus_names_.end(); ++it) {
                const Bus *bus = catalogue_.FindBus(*it);
                if (!bus->stops.empty()) { // Маршруты, на которых нет остановок, рисоваться не должны.
                    svg::Text bus_text;
                    bus_text.SetOffset(draw_settings_.bus_label_offset);
                    bus_text.SetFontSize(static_cast<uint32_t>(draw_settings_.bus_label_font_size));
                    bus_text.SetFontFamily("Verdana"s);
                    bus_text.SetFontWeight("bold"s);
                    bus_text.SetData(*it);
                    bus_text.SetPosition(projector(bus->stops.at(0)->location));

                    map_picture_.Add(svg::Text{bus_text} // подложка
                                .SetFillColor(draw_settings_.underlayer_color)
                                .SetStrokeColor(draw_settings_.underlayer_color)
                                .SetStrokeWidth(draw_settings_.underlayer_width)
                                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                            );
                    map_picture_.Add(svg::Text{bus_text}.SetFillColor(draw_settings_.color_palette.at(color_idx))); // надпись

                    if(!bus->is_roundtrip_) { // Если маршрут не кольцевой
                        const Stop *stop2 = bus->stops.at(bus->stops.size() / 2);
                        if(*stop2 != *(bus->stops.at(0))) { // для хитрых не кольцевых маршрутов но с одинаковыми остановками на концах (в JSON)
                            bus_text.SetPosition(projector(stop2->location));
                            map_picture_.Add(svg::Text{bus_text} // подложка
                                    .SetFillColor(draw_settings_.underlayer_color)
                                    .SetStrokeColor(draw_settings_.underlayer_color)
                                    .SetStrokeWidth(draw_settings_.underlayer_width)
                                    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                                    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                                );
                            map_picture_.Add(svg::Text{bus_text}.SetFillColor(draw_settings_.color_palette.at(color_idx))); // надпись
                        }
                    }

                    color_idx = (color_idx < palette_color_last) ? color_idx + 1 : 0;
                }
            }
        }

        void MapRenderer::DrawStopCircles(const geo::SphereProjector &projector) {
            using namespace transport::catalogue;
            using namespace std::literals;
            svg::Circle circle;
            circle.SetFillColor("white"s);
            circle.SetRadius(draw_settings_.stop_radius);

            for (auto it = all_stop_names_.begin(); it != all_stop_names_.end(); ++it) {
                const Stop *stop = catalogue_.FindStop(*it);
                if(stop != nullptr) {
                    map_picture_.Add(svg::Circle{circle}.SetCenter(projector(stop->location)));
                }
            }
        }

        void MapRenderer::DrawStopNames(const geo::SphereProjector &projector) {
            using namespace transport::catalogue;
            using namespace std::literals;

            svg::Text stop_text;
            stop_text.SetOffset(draw_settings_.stop_label_offset);
            stop_text.SetFontSize(static_cast<uint32_t>(draw_settings_.stop_label_font_size));
            stop_text.SetFontFamily("Verdana"s);

            for (auto it = all_stop_names_.begin(); it != all_stop_names_.end(); ++it) {
                const Stop *stop = catalogue_.FindStop(*it);
                if (stop != nullptr) {
                    stop_text.SetPosition(projector(stop->location));
                    stop_text.SetData(stop->name);
                    map_picture_.Add(svg::Text{stop_text}  // подложка
                                        .SetFillColor(draw_settings_.underlayer_color)
                                        .SetStrokeColor(draw_settings_.underlayer_color)
                                        .SetStrokeWidth(draw_settings_.underlayer_width)
                                        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                                        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                                    );
                    map_picture_.Add(svg::Text{stop_text}.SetFillColor("black"s));
                }
            }
        }

    } // namespace map_renderer
} // namespace transport