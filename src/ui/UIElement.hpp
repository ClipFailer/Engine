/**
 * @file UIElement.hpp
 * @brief DOM node element for HTML/CSS game user interfaces.
 */

#pragma once

#include "UIStyle.hpp"
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace Engine {

    /**
     * @class UIElement
     * @brief Represents a single HTML DOM element in the UI hierarchy.
     */
    class UIElement : public std::enable_shared_from_this<UIElement> {
    public:
        UIElement(std::string tag = "div") : Tag(std::move(tag)) {}

        std::string Tag{"div"};
        std::string Id;
        std::string ClassName;
        std::string Text;
        UIStyle Style;

        std::vector<std::shared_ptr<UIElement>> Children;
        std::weak_ptr<UIElement> Parent;

        std::function<void()> OnClick;

        /**
         * @brief Appends a child element to this node.
         * @param child Shared pointer to child UIElement.
         */
        void AddChild(std::shared_ptr<UIElement> child) {
            child->Parent = shared_from_this();
            Children.push_back(std::move(child));
        }

        /**
         * @brief Finds first element by HTML id attribute.
         * @param id Element id.
         * @return Shared pointer to matching element or nullptr.
         */
        std::shared_ptr<UIElement> FindById(const std::string& id) {
            if (Id == id) return shared_from_this();
            for (auto& child : Children) {
                if (auto found = child->FindById(id)) {
                    return found;
                }
            }
            return nullptr;
        }
    };

} // namespace Engine
