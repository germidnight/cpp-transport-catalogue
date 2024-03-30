/*
 * Разработать систему хранения транспортных маршрутов и обработки запросов к ней.
 * Сначала на вход подаются запросы на создание базы данных, затем — запросы к самой базе.
 */

#include <iostream>
#include <string>

#include "input_reader.h"
#include "stat_reader.h"

using namespace std;
using namespace transport::catalogue;
using namespace transport::input_reader;
using namespace transport::stat_reader;

int main() {
    TransportCatalogue catalogue;

    int base_request_count;
    cin >> base_request_count >> ws;

    {
        InputReader reader;
        for (int i = 0; i < base_request_count; ++i) {
            string line;
            getline(cin, line);
            reader.ParseLine(line);
        }
        reader.ApplyCommands(catalogue);
    }

    int stat_request_count;
    cin >> stat_request_count >> ws;
    for (int i = 0; i < stat_request_count; ++i) {
        string line;
        getline(cin, line);
        ParseAndPrintStat(catalogue, line, cout);
    }
}