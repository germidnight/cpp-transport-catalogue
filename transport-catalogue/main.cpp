/*
 * Разработать систему хранения транспортных маршрутов и обработки запросов к ней.
 * Сначала на вход подаются запросы на создание базы данных, затем — запросы к самой базе.
 */

#include <iostream>

#include "input_reader.h"
#include "stat_reader.h"

int main() {
    transport::catalogue::TransportCatalogue catalogue;

    int base_request_count;
    std::cin >> base_request_count >> std::ws;
    {
        transport::input_reader::InputReader reader;
        reader.FillTransportCatalogue(std::cin, base_request_count, catalogue);
    }

    int stat_request_count;
    std::cin >> stat_request_count >> std::ws;
    transport::stat_reader::ProcessRequests2Catalogue(std::cin, std::cout, stat_request_count, catalogue);
}