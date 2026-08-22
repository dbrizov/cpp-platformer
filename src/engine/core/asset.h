#pragma once

#include <string>

namespace hob {
    class Asset {
        std::string m_name;

    protected:
        Asset(const Asset&) = default;
        Asset& operator=(const Asset&) = default;

        Asset(Asset&&) = default;
        Asset& operator=(Asset&&) = default;

    public:
        Asset() = default;
        virtual ~Asset() = default;

        const std::string& get_name() const;
        void set_name(std::string name);
    };
} // namespace hob
