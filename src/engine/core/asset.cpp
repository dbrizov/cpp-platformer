#include "asset.h"

#include "logging.h"

namespace hob {
    const std::string& Asset::get_name() const {
        return m_name;
    }

    void Asset::set_name(std::string name) {
        if (!m_name.empty() && !name.empty() && m_name != name) {
            log::engine.error("Asset::set_name: '{}' is being renamed to '{}'; two definitions resolve to one object, "
                              "so only the last name will be visible",
                              m_name,
                              name);
        }

        m_name = std::move(name);
    }
} // namespace hob
