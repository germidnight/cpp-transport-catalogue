/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области (domain)
 * Операции сравнения над объектами:
 * 1) автобусных остановок
 * 2) автобусов
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