/**
 * @file ModuleManager.hpp
 * @brief Central registry and lifecycle coordinator for modular engine subsystems.
 */

#pragma once

#include "IModule.hpp"
#include <vector>
#include <memory>
#include <unordered_map>
#include <string>
#include <typeindex>

namespace Engine {

    /**
     * @class ModuleManager
     * @brief Singleton orchestrating registration, initialization, tick, and teardown of modules.
     */
    class ModuleManager {
    public:
        static ModuleManager& Get() noexcept;

        /**
         * @brief Registers and loads a new module.
         * @tparam T Module type inheriting from IModule.
         * @param args Arguments forwarded to module constructor.
         * @return Pointer to newly registered module instance.
         */
        template<typename T, typename... Args>
        T* RegisterModule(Args&&... args) {
            static_assert(std::is_base_of_v<IModule, T>, "T must derive from IModule");
            auto module = std::make_unique<T>(std::forward<Args>(args)...);
            T* ptr = module.get();
            std::string name(ptr->GetName());

            ptr->OnLoad();
            m_TypeMap[std::type_index(typeid(T))] = ptr;
            m_NamedModules[name] = ptr;
            m_Modules.push_back(std::move(module));
            return ptr;
        }

        /**
         * @brief Retrieves a registered module by its C++ type.
         * @tparam T Module type.
         * @return Pointer to module or nullptr.
         */
        template<typename T>
        [[nodiscard]] T* GetModule() const noexcept {
            auto it = m_TypeMap.find(std::type_index(typeid(T)));
            if (it != m_TypeMap.end()) {
                return static_cast<T*>(it->second);
            }
            return nullptr;
        }

        /**
         * @brief Retrieves a registered module by its string name.
         * @param name Module name.
         * @return Pointer to IModule or nullptr.
         */
        [[nodiscard]] IModule* GetModuleByName(std::string_view name) const noexcept;

        /**
         * @brief Updates all active modules.
         * @param deltaTime Elapsed frame time.
         */
        void UpdateModules(float deltaTime);

        /**
         * @brief Unloads and shuts down all modules.
         */
        void Shutdown();

        /**
         * @brief Returns list of all registered modules for inspection.
         * @return Const vector of module unique pointers.
         */
        [[nodiscard]] const std::vector<std::unique_ptr<IModule>>& GetModules() const noexcept {
            return m_Modules;
        }

    private:
        ModuleManager() = default;
        ~ModuleManager() = default;

        std::vector<std::unique_ptr<IModule>> m_Modules;
        std::unordered_map<std::type_index, IModule*> m_TypeMap;
        std::unordered_map<std::string, IModule*> m_NamedModules;
    };

} // namespace Engine
