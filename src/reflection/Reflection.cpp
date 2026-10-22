/**
 * @file Reflection.cpp
 * @brief Implementation of TypeRegistry singleton.
 */

#include "Reflection.hpp"

namespace Engine {

    TypeRegistry& TypeRegistry::Get() noexcept {
        static TypeRegistry instance;
        return instance;
    }

    void TypeRegistry::RegisterClass(ClassDescriptor desc) {
        std::string name = desc.GetName();
        m_Classes.insert_or_assign(name, std::move(desc));
    }

    const ClassDescriptor* TypeRegistry::FindClass(std::string_view name) const {
        auto it = m_Classes.find(std::string(name));
        if (it != m_Classes.end()) {
            return &it->second;
        }
        return nullptr;
    }

} // namespace Engine
