#include "json.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_router.h"

#include <iostream>

using namespace transport;

int main() {
    // Считать JSON из stdin
    json::Document document = json::Load(std::cin);

    // Построить на основе JSON базу данных транспортного справочника
    catalogue::TransportCatalogue catalogue;
    json_reader::JsonReader fill_catalogue;
    fill_catalogue.FillTransportCatalogue(document, catalogue);

    // считать настройки построения карты
    const map_renderer::RenderSettings draw_settings = fill_catalogue.FillRenderSettings(document);

    const transport_router::RouterSetting router_settings = fill_catalogue.FillRouterSettings(document);

    // Выполнить запросы к справочнику, находящиеся в массиве "stat_requests", построив JSON-массив
    const std::vector<json_reader::StatRequest> &stat_requests = fill_catalogue.FillStatRequests(document);

    request_handler::RequestHandler req_hndlr(stat_requests, catalogue, draw_settings, router_settings);

    json::Print(req_hndlr.GetStatistics(), std::cout);
}