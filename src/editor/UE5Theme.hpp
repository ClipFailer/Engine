/**
 * @file UE5Theme.hpp
 * @brief Unreal Engine 5 visual style, dark slate theme palette, and ImGui styling.
 */

#pragma once

#include <imgui.h>

namespace Engine {

    /**
     * @class UE5Theme
     * @brief Applies Unreal Engine 5 styling rules, fonts, and dark palette to Dear ImGui.
     */
    class UE5Theme {
    public:
        static void Apply() {
            ImGuiStyle& style = ImGui::GetStyle();
            ImVec4* colors = style.Colors;

            // Geometry and Rounding
            style.WindowRounding    = 4.0f;
            style.ChildRounding     = 3.0f;
            style.FrameRounding     = 3.0f;
            style.PopupRounding     = 4.0f;
            style.ScrollbarRounding = 4.0f;
            style.GrabRounding      = 3.0f;
            style.TabRounding       = 4.0f;

            style.WindowBorderSize  = 1.0f;
            style.ChildBorderSize   = 1.0f;
            style.PopupBorderSize   = 1.0f;
            style.FrameBorderSize   = 0.0f;
            style.TabBorderSize     = 0.0f;

            style.WindowPadding     = ImVec2(8.0f, 8.0f);
            style.FramePadding      = ImVec2(6.0f, 4.0f);
            style.ItemSpacing       = ImVec2(6.0f, 5.0f);
            style.ItemInnerSpacing  = ImVec2(4.0f, 4.0f);
            style.ScrollbarSize     = 12.0f;
            style.GrabMinSize       = 10.0f;

            // Unreal Engine 5 Charcoal & Slate Palette
            const ImVec4 bgDark         = ImVec4(0.11f, 0.11f, 0.12f, 1.0f); // Slate dark #1C1C1F
            const ImVec4 bgPanel        = ImVec4(0.14f, 0.14f, 0.16f, 1.0f); // Window background
            const ImVec4 bgHeader       = ImVec4(0.18f, 0.18f, 0.20f, 1.0f); // Header background
            const ImVec4 borderDark     = ImVec4(0.22f, 0.22f, 0.25f, 1.0f); // Subtle borders
            const ImVec4 textLight      = ImVec4(0.88f, 0.88f, 0.90f, 1.0f); // Text primary
            const ImVec4 textDim        = ImVec4(0.55f, 0.55f, 0.58f, 1.0f); // Text secondary

            // UE5 Slate Blue Accent
            const ImVec4 accentBlue     = ImVec4(0.06f, 0.44f, 0.75f, 1.0f); // Unreal blue #0F70C0
            const ImVec4 accentHover    = ImVec4(0.12f, 0.52f, 0.85f, 1.0f);
            const ImVec4 accentActive   = ImVec4(0.04f, 0.36f, 0.62f, 1.0f);

            colors[ImGuiCol_Text]                  = textLight;
            colors[ImGuiCol_TextDisabled]          = textDim;
            colors[ImGuiCol_WindowBg]              = bgPanel;
            colors[ImGuiCol_ChildBg]               = bgDark;
            colors[ImGuiCol_PopupBg]               = ImVec4(0.15f, 0.15f, 0.17f, 0.98f);
            colors[ImGuiCol_Border]                = borderDark;
            colors[ImGuiCol_BorderShadow]          = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

            colors[ImGuiCol_FrameBg]               = ImVec4(0.18f, 0.18f, 0.21f, 1.0f);
            colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.23f, 0.23f, 0.27f, 1.0f);
            colors[ImGuiCol_FrameBgActive]         = ImVec4(0.26f, 0.26f, 0.30f, 1.0f);

            colors[ImGuiCol_TitleBg]               = bgDark;
            colors[ImGuiCol_TitleBgActive]         = bgDark;
            colors[ImGuiCol_TitleBgCollapsed]      = bgDark;

            colors[ImGuiCol_MenuBarBg]             = bgDark;
            colors[ImGuiCol_ScrollbarBg]           = bgDark;
            colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.28f, 0.28f, 0.32f, 1.0f);
            colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.35f, 0.35f, 0.40f, 1.0f);
            colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.42f, 0.42f, 0.48f, 1.0f);

            colors[ImGuiCol_CheckMark]             = accentBlue;
            colors[ImGuiCol_SliderGrab]            = accentBlue;
            colors[ImGuiCol_SliderGrabActive]      = accentActive;

            colors[ImGuiCol_Button]                = ImVec4(0.20f, 0.20f, 0.23f, 1.0f);
            colors[ImGuiCol_ButtonHovered]         = ImVec4(0.27f, 0.27f, 0.32f, 1.0f);
            colors[ImGuiCol_ButtonActive]          = accentActive;

            colors[ImGuiCol_Header]                = bgHeader;
            colors[ImGuiCol_HeaderHovered]         = ImVec4(0.25f, 0.25f, 0.30f, 1.0f);
            colors[ImGuiCol_HeaderActive]          = accentBlue;

            colors[ImGuiCol_Separator]             = borderDark;
            colors[ImGuiCol_SeparatorHovered]      = accentBlue;
            colors[ImGuiCol_SeparatorActive]       = accentActive;

            colors[ImGuiCol_ResizeGrip]            = ImVec4(0.20f, 0.20f, 0.23f, 0.2f);
            colors[ImGuiCol_ResizeGripHovered]     = accentHover;
            colors[ImGuiCol_ResizeGripActive]      = accentActive;

            colors[ImGuiCol_Tab]                   = ImVec4(0.13f, 0.13f, 0.15f, 1.0f);
            colors[ImGuiCol_TabHovered]            = accentHover;
            colors[ImGuiCol_TabSelected]           = ImVec4(0.18f, 0.18f, 0.21f, 1.0f);
            colors[ImGuiCol_TabDimmed]             = ImVec4(0.11f, 0.11f, 0.13f, 1.0f);
            colors[ImGuiCol_TabDimmedSelected]     = ImVec4(0.15f, 0.15f, 0.17f, 1.0f);

            colors[ImGuiCol_DockingPreview]        = ImVec4(accentBlue.x, accentBlue.y, accentBlue.z, 0.45f);
            colors[ImGuiCol_DockingEmptyBg]        = bgDark;
        }
    };

} // namespace Engine
