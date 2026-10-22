/**
 * @file Reflection.hpp
 * @brief Runtime reflection system and macros for exposing C++ classes to the editor.
 */

#pragma once

#include "Property.hpp"
#include "scene/Object.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>

namespace Engine {

    /**
     * @class ClassDescriptor
     * @brief Runtime type description of a reflected class.
     */
    class ClassDescriptor {
    public:
        ClassDescriptor(std::string name, std::string superName = "")
            : m_Name(std::move(name)), m_SuperName(std::move(superName)) {}

        [[nodiscard]] const std::string& GetName() const noexcept { return m_Name; }
        [[nodiscard]] const std::string& GetSuperName() const noexcept { return m_SuperName; }
        [[nodiscard]] const std::vector<PropertyDescriptor>& GetProperties() const noexcept { return m_Properties; }

        /**
         * @brief Adds a reflected property to the class descriptor.
         * @param prop Property descriptor.
         */
        void AddProperty(PropertyDescriptor prop) {
            m_Properties.push_back(std::move(prop));
        }

        /**
         * @brief Sets factory instantiation function for this class.
         * @param factory Callable returning unique_ptr<Object>.
         */
        void SetFactory(std::function<std::unique_ptr<Object>()> factory) {
            m_Factory = std::move(factory);
        }

        /**
         * @brief Instantiates a new instance of this class.
         * @return Unique pointer to newly allocated object.
         */
        [[nodiscard]] std::unique_ptr<Object> CreateInstance() const {
            if (m_Factory) {
                return m_Factory();
            }
            return nullptr;
        }

    private:
        std::string m_Name;
        std::string m_SuperName;
        std::vector<PropertyDescriptor> m_Properties;
        std::function<std::unique_ptr<Object>()> m_Factory;
    };

    /**
     * @class TypeRegistry
     * @brief Singleton registry holding all reflected classes and types in the engine.
     */
    class TypeRegistry {
    public:
        /**
         * @brief Retrieves the singleton instance.
         * @return Reference to TypeRegistry.
         */
        static TypeRegistry& Get() noexcept;

        /**
         * @brief Registers a class descriptor.
         * @param desc Class descriptor to add.
         */
        void RegisterClass(ClassDescriptor desc);

        /**
         * @brief Finds class descriptor by name.
         * @param name Name of class.
         * @return Pointer to ClassDescriptor or nullptr if not found.
         */
        [[nodiscard]] const ClassDescriptor* FindClass(std::string_view name) const;

        /**
         * @brief Returns map of all registered classes.
         * @return Map of class names to descriptors.
         */
        [[nodiscard]] const std::unordered_map<std::string, ClassDescriptor>& GetAllClasses() const noexcept {
            return m_Classes;
        }

    private:
        TypeRegistry() = default;
        std::unordered_map<std::string, ClassDescriptor> m_Classes;
    };

} // namespace Engine

/**
 * @def GENERATED_BODY()
 * @brief Macro placed inside class header to inject reflection methods.
 */
#define GENERATED_BODY() \
public: \
    [[nodiscard]] virtual std::string_view GetClassName() const override; \
    [[nodiscard]] static std::string_view StaticClassName(); \
    static void RegisterReflection();

/**
 * @def ENGINE_CLASS_BEGIN(ClassType, SuperType)
 * @brief Begins reflection definition for a class in a source file.
 */
#define ENGINE_CLASS_BEGIN(ClassType, SuperType) \
    std::string_view ClassType::GetClassName() const { return #ClassType; } \
    std::string_view ClassType::StaticClassName() { return #ClassType; } \
    void ClassType::RegisterReflection() { \
        ::Engine::ClassDescriptor desc(#ClassType, #SuperType); \
        desc.SetFactory([]() -> std::unique_ptr<::Engine::Object> { \
            return std::make_unique<ClassType>(); \
        });

/**
 * @def ENGINE_PROPERTY(ClassType, MemberVar, TypeKind, Category, Tooltip)
 * @brief Exposes a member variable to reflection and editor inspector.
 */
#define ENGINE_PROPERTY(ClassType, MemberVar, TypeKind, Category, Tooltip) \
        desc.AddProperty(::Engine::PropertyDescriptor( \
            #MemberVar, \
            ::Engine::PropertyType::TypeKind, \
            offsetof(ClassType, MemberVar), \
            Category, \
            Tooltip \
        ));

/**
 * @def ENGINE_CLASS_END()
 * @brief Finalizes reflection definition and registers with TypeRegistry.
 */
#define ENGINE_CLASS_END() \
        ::Engine::TypeRegistry::Get().RegisterClass(std::move(desc)); \
    }
