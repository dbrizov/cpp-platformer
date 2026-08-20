#pragma once

#include <functional>
#include <string>
#include <string_view>

namespace hob {
    // Transparent hash for std::string-keyed maps. Paired with std::equal_to<>, it lets
    // find/contains take a std::string_view or const char* without materializing a
    // temporary std::string for the lookup.
    struct StringHash {
        using is_transparent = void;

        size_t operator()(const char* s) const {
            return std::hash<std::string_view>{}(s);
        }

        size_t operator()(std::string_view s) const {
            return std::hash<std::string_view>{}(s);
        }

        size_t operator()(const std::string& s) const {
            return std::hash<std::string_view>{}(s);
        }
    };
} // namespace hob
