/**
 * @file CustomComponent.hpp
 * @brief Dynamic gameplay component enabling arbitrary custom components with reflected properties.
 */

#pragma once

#include "Component.hpp"
#include <string>

namespace Engine {

    /**
     * @class CustomComponent
     * @brief Dynamic extensible component allowing user-defined custom behaviors and properties.
     */
    class CustomComponent : public Component {
        GENERATED_BODY()
    public:
        CustomComponent();
        explicit CustomComponent(std::string typeName);
        ~CustomComponent() override = default;

        void BeginPlay() override;
        void Tick(float deltaTime) override;

        std::string ComponentType{"CustomComponent"};
        float CustomScalar{100.0f};
        bool bIsEnabled{true};
        std::string CustomTag{"GameplayData"};
    };

} // namespace Engine
