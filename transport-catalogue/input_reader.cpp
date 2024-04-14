/* обработка запросов на заполнение справочника */
#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <utility>

using namespace transport::catalogue;
using namespace transport::detail;

namespace transport {
namespace input_reader {

/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
 */
Coordinates ParseCoordinates(std::string_view str) {
    static const double nan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == str.npos) {
        return {nan, nan};
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);

    double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));

    comma = str.find(',', not_space2);
    double lng;
    if(comma != str.npos) {
        lng = std::stod(std::string(str.substr(not_space2, comma - not_space2)));
    } else {
        lng = std::stod(std::string(str.substr(not_space2)));
    }

    return {lat, lng};
}

/**
 * Разбирает строку вида "55.595884, 37.209755, 9900m to Rasskazovka, 100m to Marushkino"
 * Возвращает вектор пар: <Имя остановки, расстояние до остановки>
 */
std::vector<std::pair<std::string, size_t>> ParseDistancesToStop(std::string_view str) {
    using namespace std::string_literals;

    const std::string dist_delim = "m to"s;
    std::string stop_name_to;
    std::vector<std::pair<std::string, size_t>> result;

    size_t comma = str.find(',');
    comma = str.find(',', comma + 1);
    size_t not_space = comma;
    while(not_space != str.npos) {
        not_space = str.find_first_not_of(' ', comma + 1);
        size_t next_pos = str.find(dist_delim, not_space);
        size_t distance = std::stoul(std::string(str.substr(not_space, next_pos - not_space)));

        if(next_pos != str.npos) {
            not_space = str.find_first_not_of(' ', next_pos + dist_delim.size());
        } else {
            not_space = next_pos;
            break;
        }
        comma = str.find(',', not_space);
        if(comma != str.npos) {
            stop_name_to = std::string(str.substr(not_space, comma - not_space));
        } else {
            stop_name_to = std::string(str.substr(not_space));
        }
        result.push_back({stop_name_to, distance});
        not_space = comma;
    }
    return result;
}

/**
 * Удаляет пробелы в начале и конце строки
 */
std::string_view Trim(std::string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
 */
std::vector<std::string_view> Split(std::string_view string, char delim) {
    std::vector<std::string_view> result;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == string.npos) {
            delim_pos = string.size();
        }
        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }

    return result;
}

/**
 * Парсит маршрут.
 * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
 * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
 */
std::vector<std::string_view> ParseRoute(std::string_view route) {
    if (route.find('>') != route.npos) {
        return Split(route, '>');
    }

    auto stops = Split(route, '-');
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return results;
}

CommandDescription ParseCommandDescription(std::string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }

    return {std::string(line.substr(0, space_pos)),
            std::string(line.substr(not_space, colon_pos - not_space)),
            std::string(line.substr(colon_pos + 1))};
}

void InputReader::ParseLine(std::string_view line) {
    auto command_description = ParseCommandDescription(line);
    if (command_description) {
        commands_.push_back(std::move(command_description));
    }
}

void InputReader::ApplyCommands([[maybe_unused]] TransportCatalogue &catalogue) const {
    size_t idx = 0;
    std::vector<size_t> bus_cmd_idxs, dist_cmd_idxs;

    for(auto& cmd : commands_) {
        if(cmd.command == "Stop") {
            auto coord = ParseCoordinates(cmd.description);
            catalogue.AddStop(std::move(cmd.id), coord.lat, coord.lng);
            dist_cmd_idxs.push_back(idx);
        }
        if(cmd.command == "Bus") {
            bus_cmd_idxs.push_back(idx);
        }
        ++idx;
    }

    for (size_t i : dist_cmd_idxs) {
        catalogue.AddStopDistances(commands_[i].id, std::move(ParseDistancesToStop(commands_[i].description)));
    }

    for(size_t i : bus_cmd_idxs) {
        catalogue.AddBus(commands_[i].id, ParseRoute(commands_[i].description));
    }
}

void InputReader::FillTransportCatalogue(std::istream& input, int base_request_count, TransportCatalogue& catalogue) {
    for (int i = 0; i < base_request_count; ++i) {
        std::string line;
        std::getline(input, line);
        ParseLine(line);
    }
    ApplyCommands(catalogue);
}

} // конец namespace input_reader
} // конец namespace transport