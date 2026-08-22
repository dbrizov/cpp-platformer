#pragma once

#include <string>

namespace hob {
    class Asset {
        std::string m_name;

    public:
        virtual ~Asset() = default;

        const std::string& get_name() const;
        void set_name(std::string name);
    };
} // namespace hob
