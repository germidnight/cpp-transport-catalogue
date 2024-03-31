/* обработка запросов на заполнение справочника
 * Формат ввода базы данных.
 * В первой строке стандартного потока ввода содержится число N — количество запросов на обновление базы данных,
 * затем — по одному на строке — вводятся сами запросы. Запросы бывают двух типов:
 * 1) Stop X: latitude, longitude - Добавляет информацию об остановке с названием X и координатами latitude (широта) и longitude (долгота) на земной поверхности.
 * Название остановки может состоять из нескольких слов. Используйте двоеточие как признак окончания названия остановки.
 * Широта задаётся в градусах от -90.0 (южный полюс) до +90.0 (северный полюс). Положительные широты расположены севернее экватора, отрицательные — южнее.
 * Долгота задаётся в градусах от -180.0 до +180.0, положительные значения соответствуют восточной долготе, а отрицательные — западной.
 * Нулевой меридиан проходит через Гринвичскую королевскую обсерваторию в Лондоне, а координаты останкинской телевышки равны 55.8199081 северной широты и 37.6116028 восточной долготы.
 * Широта и долгота разделяются запятой, за которой следует пробел. Гарантируется, что остановка X определена не более чем в одном запросе Stop.
 * Пример 'Stop Biryulyovo Tovarnaya: 55.592028, 37.653656'
 * 2) Bus X: описание маршрута - Добавление автобусного маршрута X. Название маршрута может состоять из нескольких слов и отделяется от описания двоеточием.
 * Описание маршрута может задаваться в одном из двух форматов (см. пример):
 * а) stop1 - stop2 - ... stopN: автобус следует от stop1 до stopN и обратно с указанными промежуточными остановками. Пример 'Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka'
 * б) stop1 > stop2 > ... > stopN > stop1: кольцевой маршрут с конечной stop1. Пример 'Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye'
 * Гарантируется, что каждая из остановок маршрута определена в некотором запросе Stop, а сам маршрут X определён не более чем в одном запросе Bus.
 * Запросы Stop и Bus могут идти в любом порядке. В запросе Bus могут быть перечислены названия остановок, которые будут объявлены позднее.
 */

#pragma once

#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "geo.h"
#include "transport_catalogue.h"

using namespace transport::catalogue;

namespace transport {
namespace input_reader {

struct CommandDescription {
    // Определяет, задана ли команда (поле command непустое)
    explicit operator bool() const {
        return !command.empty();
    }

    bool operator!() const {
        return !operator bool();
    }

    std::string command;     // Название команды
    std::string id;          // id маршрута или остановки
    std::string description; // Параметры команды
};

class InputReader {
public:
    /**
     * Парсит строку в структуру CommandDescription и сохраняет результат в commands_
     */
    void ParseLine(std::string_view line);

    /**
     * Наполняет данными транспортный справочник, используя команды из commands_
     */
    void ApplyCommands(TransportCatalogue &catalogue) const;

    void FillTransportCatalogue(std::istream& input, int base_request_count, TransportCatalogue& catalogue);

private:
    std::vector<CommandDescription> commands_;
};
} // конец namespace input_reader
} // конец namespace transport