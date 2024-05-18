/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области (domain)
 * вашего приложения и не зависят от транспортного справочника. Например Автобусные маршруты и Остановки.
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 */

#include "domain.h"

namespace transport {
    namespace catalogue {
        [[nodiscard]] bool Stop::operator!=(const Stop &rhs) const noexcept {
            return (location.lat != rhs.location.lat) || (location.lng != rhs.location.lng) || (name != rhs.name);
        }
        [[nodiscard]] bool Stop::operator==(const Stop &rhs) const noexcept {
            return !(*this != rhs);
        }

        [[nodiscard]] bool Bus::operator!=(const Bus &rhs) const noexcept {
            if (name != rhs.name) {
                return true;
            }
            if (stops.size() != rhs.stops.size()) {
                return true;
            }
            auto it_rhs = rhs.stops.begin();
            for (auto it_lhs = stops.begin(); it_lhs != stops.end(); ++it_lhs, ++it_rhs) {
                if ((*it_lhs)->name != (*it_rhs)->name) {
                    return true;
                }
            }
            return false;
        }
        [[nodiscard]] bool Bus::operator==(const Bus &rhs) const noexcept {
            return !(*this != rhs);
        }
    } // namespace catalogue
} // namespace transport