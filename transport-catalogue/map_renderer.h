#pragma once

#include "svg.h"
#include "transport_catalogue.h"

namespace transport {
    namespace map_renderer {

        struct RenderSettings
        {
            double width = 0;
            double height = 0;
            double padding = 0;
            double line_width = 0;
            double stop_radius = 0;
            int bus_label_font_size = 0;
            svg::Point bus_label_offset;
            int stop_label_font_size = 0;
            svg::Point stop_label_offset;
            svg::Color underlayer_color;
            double underlayer_width = 0;
            std::vector<svg::Color> color_palette;
        };

        class MapRenderer {
        public:
            MapRenderer(const catalogue::TransportCatalogue &catalogue, const RenderSettings &draw_settings)
                        : catalogue_(catalogue), draw_settings_(draw_settings) {}
            svg::Document& DrawMap();

        private:
            void DrawBusLines(const geo::SphereProjector &projector);
            void DrawBusNames(const geo::SphereProjector &projector);
            void DrawStopCircles(const geo::SphereProjector &projector);
            void DrawStopNames(const geo::SphereProjector &projector);

            const catalogue::TransportCatalogue &catalogue_;
            const RenderSettings &draw_settings_;
            svg::Document map_picture_;
            std::vector<std::string> all_stop_names_; // отсортированный список остановок, через которые проходят маршруты
            std::vector<std::string> all_bus_names_;  // отсортированный список маршрутов
        };

    } // namespace map_renderer
} // namespace transport