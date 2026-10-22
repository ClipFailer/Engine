/**
 * @file ModuleManager.cpp
 * @brief Implementation of ModuleManager singleton.
 */

#include "ModuleManager.hpp"
#include "core/Log.hpp"

namespace Engine {

    ModuleManager& ModuleManager::Get() noexcept {
        static ModuleManager instance;
        return instance;
    }

    IModule* ModuleManager::GetModuleByName(std::string_view name) const noexcept {
        auto it = m_NamedModules.find(std::string(name));
        if (it != m_NamedModules.end()) {
            return it->second;
        }
        return nullptr;
    }

    void ModuleManager::UpdateModules(float deltaTime) {
        for (auto& module : m_Modules) {
            if (module && module->IsEnabled()) {
                module->OnUpdate(deltaTime);
            }
        }
    }

    void ModuleManager::Shutdown() {
        for (auto it = m_Modules.rbegin(); it != m_Modules.rend(); ++it) {
            if (*it) {
                LOG_INFO("Unloading module: {}", (*it)->GetName());
                (*it)->OnUnload();
            }
        }
        m_Modules.clear();
        m_TypeMap.clear();
        m_NamedModules.clear();
    }

} // namespace Engine
